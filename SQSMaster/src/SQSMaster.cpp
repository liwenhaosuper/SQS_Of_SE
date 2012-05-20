/*
 * SQSMaster.cpp
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 */

#include "SQSMaster.h"
#include <iostream>
#include <err.h>

#include "event.h"
#include "evhttp.h"
#include "event2/http.h"
#include <sys/time.h>
#include "Convention.h"
#include <string.h>


#include <cstdlib>
using namespace std;


void DataNode::Recycle(){
	time_t cur;
	time(&cur);
	startTime = 1000*((long)cur);
}
bool DataNode::IsAlive(){
	time_t cur;
	time(&cur);
	if(1000*((long)cur)<(startTime+defaultTimeout)){
		return true;
	}
	return false;
}

void* RegularCheck(void* arg){
	if(arg==NULL){
		cout<<"arg is null..."<<endl;
		return (void*)0;
	}
	Param* param = (Param*) arg;
	if(param==NULL){
		cout<<"param is null"<<endl;
	}
	HeartBeat* instance = (HeartBeat*)param->instance;
	DataNode* node = param->node;
	if(instance==NULL){
		cout<<"instance is null"<<endl;
	}
	int cnt = 0;
	while(true){
		struct evhttp_request *rsp = instance->doRequest(node->getNodeNameFormaster(),node->getNodePortFormaster());
		if(rsp==NULL){
			cout<<"Rsp is NULL..."<<endl;
			cnt++;
		}else if(rsp->response_code==0){
			cnt++;
		}else{
			cnt = 0;
		}
		if(cnt>5){
			instance->CancelDataNode(*node);
			break;
		}
		if(node==NULL||node->IsCancel()){
			break;
		}
		if(cnt==0){
			node->Recycle();
		}
		sleep(instance->cycle/1000);
	}
	return (void*)0;
}

void heartbeat_callback(struct evhttp_request *req, void *rsp){
//	cout<<"heartbeat callback"<<":rsp body size:"<<req->body_size<<":rsp code line:"<<req->response_code_line<<":"<<
//			"rsp header size:"<<req->headers_size<<endl;
//	cout<<rsp<<endl;
	/**
	 * well, it takes me a long time to find you,bug!
	 * so there is one question: if the pointer is modified by another thread,how could
	 * the original thread get the newest value? seems using keyword 'volatile' doesn't work
	 */
	memcpy(rsp,req,sizeof(struct evhttp_request));
	//rsp = req;
//	cout<<rsp<<endl;
//	cout<<":rsp body size:"<<((struct evhttp_request *)rsp)->body_size
//			<<":rsp code line:"<<((struct evhttp_request *)rsp)->response_code_line
//			<<":rsp header size:"<<((struct evhttp_request *)rsp)->headers_size<<endl;
}

struct evhttp_request* HeartBeat::doRequest(std::string dataNode,int port){
	struct event_base *base = event_base_new();
	struct evhttp_connection *cn = evhttp_connection_base_new(
	        base, NULL,
	        dataNode.c_str(),
	        port);
	struct evhttp_request *rsp = NULL;
	/* Allocate request structure */
	if ((rsp = (struct evhttp_request *)malloc(sizeof(struct evhttp_request))) == NULL) {
		cout<<"Error allocating rsp structure"<<endl;
		return NULL;
	}
	//cout<<"original rsp:"<<rsp<<endl;
	struct evhttp_request *req = evhttp_request_new(heartbeat_callback,rsp);
	evhttp_make_request(cn,req,EVHTTP_REQ_GET,HEARTBEAT.c_str());
	event_base_dispatch(base);
//	cout<<"sent heart beat:"<<rsp<<endl;
//	cout<<"rsp::::rsp body size:"<<((struct evhttp_request *)rsp)->body_size
//			<<":rsp code line:"<<((struct evhttp_request *)rsp)->response_code_line
//			<<":rsp header size:"<<((struct evhttp_request *)rsp)->headers_size<<endl;
	return (struct evhttp_request *)rsp;
}

void HeartBeat::AddDataNode(DataNode node){
	//cancel the existing same node
	for(vector<DataNode*>::iterator iter = clients.begin();iter!=clients.end();iter++){
		if(node==(**iter)){
			CancelDataNode(**iter);
			break;
		}
	}
	DataNode* newNode = new DataNode(node);
	newNode->Recycle();
	clients.push_back(newNode);

	Param* param = new Param;
	param->instance = this;
	param->node = newNode;
	pthread_t pid;
	if(pthread_create(&pid,NULL,RegularCheck,param)!=0){
		cout<<"Error in starting heart beat..."<<endl;
	}
}
bool HeartBeat::IsNodeAlive(DataNode node){
	for(vector<DataNode*>::iterator iter = clients.begin();iter!=clients.end();iter++){
		if(node==(**iter)){
			if((*iter)->IsAlive()){
				return true;
			}
			CancelDataNode(**iter);
			return false;
		}
	}
	return false;
}
void HeartBeat::CancelDataNode(DataNode node){
	for(vector<DataNode*>::iterator iter = clients.begin();iter!=clients.end();iter++){
		if(node==(**iter)){
			(*iter)->Cancel();
			clients.erase(iter);
			break;
		}
	}
}


void ClientCallBack(struct evhttp_request* req, void* arg){
	SQSMaster* obj = (SQSMaster*)arg;
	obj->onClientReqRecv(req);
}

void DataNodeCallBack(struct evhttp_request* req, void* arg){
	SQSMaster* obj = (SQSMaster*)arg;
	obj->onDataNodeRecv(req);
}

void dispatchMsgCallBack(struct evhttp_request* req, void* arg){
	//event_base_loopbreak((struct event_base*)arg);
	//cout<<"dispatchMsgCallBack:Care about nothing!"<<endl;
}
//FIXME and IMPORTANT: there is a bug. It will block for a long time sometimes
void SQSMaster::dispatchMessage(std::string remoteNode,int remotePort,std::string request){
	struct event_base *base = event_base_new();
	struct evhttp_connection *cn = evhttp_connection_base_new(
			base, NULL,
			remoteNode.c_str(),
			remotePort);
	struct evhttp_request *req = evhttp_request_new(dispatchMsgCallBack, base);
	if(evhttp_make_request(cn,req,EVHTTP_REQ_GET,request.c_str())==-1){
		cout<<"Make request fail..."<<endl;
	}
	event_base_dispatch(base);
}

void SQSMaster::onDataNodeRecv (struct evhttp_request* req) {
	cout<<"Receive msg from data node.path:"<<req->uri<<endl;
	struct evbuffer *buf;
	buf = evbuffer_new();
	/* parse path and URL paramter */
	struct evkeyvalq *url_parameters = new evkeyvalq;
	const char* url_path = evhttp_uri_get_path(req->uri_elems);
	const char* url_query = evhttp_uri_get_query(req->uri_elems);
	evhttp_parse_query_str(url_query,url_parameters);

	if(url_parameters==NULL){
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		return;
	}
	const char* nodeName = evhttp_find_header(url_parameters,NODE_NAME.c_str());
	const char* nodePort = evhttp_find_header(url_parameters,NODE_PORT.c_str());
	if(nodeName==NULL||nodePort==NULL){
		evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/operation?nodeName=value1&nodePort=value2&...");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		if(nodeName) delete nodeName;
		if(nodePort) delete nodePort;
		return;
	}
	/*create queue*/
	if(strcmp(url_path,CREATE_QUEUE.c_str())==0){
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s","The request url should be this kind:http:hostName:hostPort/createQueue?nodeName=value1&nodePort=value2&queueName=value3");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		// add log
		string command = CREATE_QUEUE;
		command+="?"+QUEUE_NAME+"="+queueName;
		logger->addLog(command);
		//dispatch message
		for(vector<DataNode>::iterator iter = mDataNodes.begin();iter!=mDataNodes.end();iter++){
			if(!pBeat->IsNodeAlive(*iter)){
				 iter = mDataNodes.erase(iter);
				 if(iter==mDataNodes.end()){
					 break;
				 }
			}
			if(strcmp(nodeName,(iter)->getNodeNameFormaster().c_str())!=0&&atoi(nodePort)!=(iter)->getNodePortFormaster()){
				dispatchMessage((iter)->getNodeNameFormaster(),(iter)->getNodePortFormaster(),command);
			}
		}
		evbuffer_add_printf(buf, "%s", "Create Queue Request Accepted!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		delete queueName;
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		return;
	}else if(strcmp(url_path,DEL_QUEUE.c_str())==0){/*delete queue*/
		// delete queue
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/deleteQueue?nodeName=value1&nodePort=value2&queueName=value3");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		// add log
		string command = DEL_QUEUE;
		command+="?"+QUEUE_NAME+"="+queueName;
		logger->addLog(command);

		//dispatch message
		for(vector<DataNode>::iterator iter = mDataNodes.begin();iter!=mDataNodes.end();iter++){
			if(!pBeat->IsNodeAlive(*iter)){
				 iter = mDataNodes.erase(iter);
				 if(iter==mDataNodes.end()){
					 break;
				 }
			}

			if(strcmp(nodeName,(iter)->getNodeNameFormaster().c_str())!=0&&atoi(nodePort)!=(iter)->getNodePortFormaster()){
				dispatchMessage((iter)->getNodeNameFormaster(),(iter)->getNodePortFormaster(),command);
			}
		}
		evbuffer_add_printf(buf, "%s", "Delete Queue Request Accepted!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		delete queueName;
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		return;
	}else if(strcmp(url_path,PUT_MSG.c_str())==0){
		//put message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/putMessage?nodeName=value1&nodePort=value2&queueName=value3&message=val4&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		const char* msgId = evhttp_find_header(url_parameters,MSG_ID.c_str());
		const char* msg = evhttp_find_header(url_parameters,MSG.c_str());
		if(msg==NULL||msgId==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/putMessage?nodeName=value1&nodePort=value2&queueName=value3&message=val4&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			delete queueName;
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			if(msgId)delete msgId;
			if(msg)delete msg;
			return;
		}
		//add log
		string command = PUT_MSG;
		command+="?"+QUEUE_NAME+"="+queueName+"&"+MSG+"="+msg+"&"+MSG_ID+"="+msgId;
		logger->addLog(command);
		//dispatch message
		for(vector<DataNode>::iterator iter = mDataNodes.begin();iter!=mDataNodes.end();iter++){
			if(!pBeat->IsNodeAlive(*iter)){
				 iter = mDataNodes.erase(iter);
				 if(iter==mDataNodes.end()){
					 break;
				 }
			}
			if(strcmp(nodeName,(iter)->getNodeNameFormaster().c_str())!=0&&atoi(nodePort)!=(iter)->getNodePortFormaster()){
				dispatchMessage((iter)->getNodeNameFormaster(),(iter)->getNodePortFormaster(),command);
			}
		}
		evbuffer_add_printf(buf, "%s", "Put message Request Accepted!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		delete queueName;
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		delete msgId;
		delete msg;
		return;
	}else if(strcmp(url_path,DELETE_MSG.c_str())==0){
		//delete message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/deleteMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		const char* msgId = evhttp_find_header(url_parameters,MSG_ID.c_str());
		if(msgId==NULL){
			evbuffer_add_printf(buf,"%s","The request url should be this kind:http:hostName:hostPort/deleteMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			delete queueName;
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		//add log
		string command = DELETE_MSG;
		command+="?"+QUEUE_NAME+"="+queueName+"&"+MSG_ID+"="+msgId;
		logger->addLog(command);
		//dispatch message
		for(vector<DataNode>::iterator iter = mDataNodes.begin();iter!=mDataNodes.end();iter++){
			if(!pBeat->IsNodeAlive(*iter)){
				 iter = mDataNodes.erase(iter);
				 if(iter==mDataNodes.end()){
					 break;
				 }
			}
			if(strcmp(nodeName,(iter)->getNodeNameFormaster().c_str())!=0&&atoi(nodePort)!=(iter)->getNodePortFormaster()){
				dispatchMessage((iter)->getNodeNameFormaster(),(iter)->getNodePortFormaster(),command);
			}
		}
		//unlock the message
		pLock->UnLock(queueName,atoi(msgId));

		evbuffer_add_printf(buf, "%s", "Delete Message Request Accepted!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		delete queueName;
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		return;

	}else if(strcmp(url_path,GET_MSG.c_str())==0){
		//get message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/getMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		const char* msgId = evhttp_find_header(url_parameters,MSG_ID.c_str());
		if(pLock->Lock(queueName,atoi(msgId))){
			evbuffer_add_printf(buf, "%s", "Success");
		}else{
			evbuffer_add_printf(buf, "%s", "Fail");
		}
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		delete queueName;
		return;
	}else if(strcmp(url_path,RECOVERY.c_str())==0){ /* recovery*/
		const char* logsize = evhttp_find_header(url_parameters,LOG_SIZE.c_str());
		if(logsize==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/recovery?nodeName=value1&nodePort=value2&logsize=val3");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			return;
		}
		int sz = logger->count()-atoi(logsize);
		if(sz<0){
			evbuffer_add_printf(buf, "%s", "Error:Logsize is larger than the master's!");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			delete logsize;
			return;
		}
		vector<string> datas =  logger->tailList(sz);
		for(vector<string>::iterator iter = datas.begin();iter!=datas.end();iter++){
			dispatchMessage(nodeName,atoi(nodePort),*iter);
		}
		evbuffer_add_printf(buf, "%s", "Recovery is completed!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		delete logsize;
		return;
	}else if(strcmp(url_path,JOIN_TEAM.c_str())==0){/*  join*/
		const char* publicNodeName = evhttp_find_header(url_parameters,PUBLIC_NODE_NAME.c_str());
		const char* publicNodePort = evhttp_find_header(url_parameters,PUBLIC_NODE_PORT.c_str());
		if(publicNodeName==NULL||publicNodePort==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/join?nodeName=value1&nodePort=value2&publicNodeName=val3&publicNodePort=val4");
			evhttp_send_reply(req, HTTP_OK, "NO", buf);
			evbuffer_free(buf);
			//delete url_path;
			//delete url_query;
			//FIXME: how should this data structure be freed?
			delete url_parameters;
			delete nodeName;
			delete nodePort;
			if(publicNodeName) delete publicNodeName;
			if(publicNodePort) delete publicNodePort;
			return;
		}
		DataNode* dNode = new DataNode(publicNodeName,atoi(publicNodePort),nodeName,atoi(nodePort),10000);
		mDataNodes.push_back(*dNode);
		pBeat->AddDataNode(*dNode);
		evbuffer_add_printf(buf, "%s", "Welcome to join us!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		delete publicNodeName;
		delete publicNodePort;
		return;
	}else if(strcmp(url_path,EMPTY_PATH.c_str())==0){
		//TODO:other opt
		evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/operation?nodeName=value1&nodePort=value2&...");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		return;
	}else{
		//TODO: hacker?
		evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/operation?nodeName=value1&nodePort=value2&...");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		//delete url_path;
		//delete url_query;
		//FIXME: how should this data structure be freed?
		delete url_parameters;
		delete nodeName;
		delete nodePort;
		return;

	}
}

void SQSMaster::onClientReqRecv (struct evhttp_request* req) {
	cout<<"Receive msg from client.path:"<<req->uri<<endl;
    struct evbuffer *buf;
    buf = evbuffer_new();

    const char *url_path;
    url_path =evhttp_uri_get_path(req->uri_elems);
	if(strcmp(url_path,GET_AVAILABLE_HOST.c_str())==0){
		int cnt = mDataNodes.size();
		if(cnt>0){
			int index = rand()%cnt;
			string res = "nodeName:"+mDataNodes.at(index).getNodeNamePublic()+"\r\n";
			char* tmp = new char[8];
			memset(tmp,0,sizeof(tmp));
			sprintf(tmp,"%d",mDataNodes.at(index).getNodePortPublic());
			res+="nodePort:";
			res+=tmp;
			res+="\r\n";
			evbuffer_add_printf(buf, "%s",res.c_str());
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
		}else{
			evbuffer_add_printf(buf, "%s", "No Data Node available");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
		}

	}else{
		evbuffer_add_printf(buf, "%s", "Unrecognized request......");
		evhttp_send_reply(req, HTTP_BADREQUEST, "NO", buf);
	}
	evbuffer_free(buf);
}

bool SQSMaster::init(){
	event_init();
	struct evhttp *httpd_client,*httpd_dataNode;
	httpd_client = evhttp_start("0.0.0.0", clientPort);
	httpd_dataNode = evhttp_start("0.0.0.0",dataNodePort);
	if(httpd_client==NULL){
		fprintf(stderr,"ERROR:Unable to listen on %s:%d\n","0.0.0.0",clientPort);
		return false;
	}
	if(httpd_dataNode==NULL){
		fprintf(stderr,"ERROR:Unable to listen on %s:%d\n","0.0.0.0",dataNodePort);
		return false;
	}
	evhttp_set_timeout(httpd_client, connectionTimeout);
	evhttp_set_timeout(httpd_dataNode, connectionTimeout);
	evhttp_set_gencb(httpd_client, ClientCallBack, this);
	evhttp_set_gencb(httpd_dataNode, DataNodeCallBack,this);
	return true;
}
void SQSMaster::start(){
	pLock->Start();
	event_dispatch();
}
