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
	cout<<"Heart beat start..."<<endl;
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
		//TODO: parse the response
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
			cout<<"node is null or canceled"<<endl;
			break;
		}
		if(cnt==0){
			cout<<"Heart beat recv..."<<endl;
			node->Recycle();
		}
		sleep(instance->cycle/1000);
	}
	return (void*)0;
}

void request_callback(struct evhttp_request *req, void *rsp){
	memcpy(rsp,req,sizeof(struct evhttp_request*));
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
	struct evhttp_request *req = evhttp_request_new(request_callback, rsp);
	evhttp_make_request(cn,req,EVHTTP_REQ_GET,HEARTBEAT.c_str());
	event_base_dispatch(base);
	return rsp;
}

void HeartBeat::AddDataNode(DataNode node){
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
				cout<<"Cancelling data node..."<<endl;
				(*iter)->Cancel();
				clients.erase(iter);
				cout<<"Cancelled data node..."<<endl;
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
	event_base_loopbreak((struct event_base*)arg);
}

void SQSMaster::dispatchMessage(std::string remoteNode,int remotePort,std::string request){
	struct event_base *base = event_base_new();
	struct evhttp_connection *cn = evhttp_connection_base_new(
			base, NULL,
			remoteNode.c_str(),
			remotePort);
	struct evhttp_request *req = evhttp_request_new(request_callback, base);
	evhttp_make_request(cn,req,EVHTTP_REQ_GET,request.c_str());
	event_base_dispatch(base);
}

void SQSMaster::onDataNodeRecv (struct evhttp_request* req) {
	cout<<"Receive msg from data node..."<<endl;
	struct evbuffer *buf;
	buf = evbuffer_new();
	/* parse path and URL paramter */
	const char *url_path;
	const char *url_query;
	struct evkeyvalq *url_parameters = new evkeyvalq;

	url_path = evhttp_uri_get_path(req->uri_elems);
	url_query = evhttp_uri_get_query(req->uri_elems);
	evhttp_parse_query_str(url_query,url_parameters);

	//for test only
//	if(url_parameters!=NULL){
//		cout<<"Query parameters are:"<<endl;
//		evkeyval* iter = url_parameters->tqh_first;
//		while(iter!=NULL){
//			cout<<iter->key<<":"<<iter->value<<endl;
//			iter = iter->next.tqe_next;
//		}
//	}

	if(url_parameters==NULL){
		cout<<"You must be a hack..."<<endl;
		evbuffer_free(buf);
		return;
	}
	const char* nodeName = evhttp_find_header(url_parameters,NODE_NAME.c_str());
	const char* nodePort = evhttp_find_header(url_parameters,NODE_PORT.c_str());
	if(nodeName==NULL||nodePort==NULL){
		evbuffer_add_printf(buf, "%s", "You may be a hack,too!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}
	/*create queue*/
	if(strcmp(url_path,CREATE_QUEUE.c_str())==0){
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName to create? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
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
		evbuffer_add_printf(buf, "%s", "Create Queue Request?Roger that");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else if(strcmp(url_path,DEL_QUEUE.c_str())==0){/*delete queue*/
		//TODO: delete queue
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName to delete? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
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
		evbuffer_add_printf(buf, "%s", "Delete Queue Request?Roger that");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else if(strcmp(url_path,LIST_QUEUES.c_str())==0){
		//TODO:list queues, well, it will not reach here
		evbuffer_add_printf(buf, "%s", "Do not let me list queues, ok?");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else if(strcmp(url_path,PUT_MSG.c_str())==0){
		//TODO:put message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName and add a message? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			return;
		}
		const char* msgId = evhttp_find_header(url_parameters,MSG_ID.c_str());
		const char* msg = evhttp_find_header(url_parameters,MSG_CONTENT.c_str());
		if(msg==NULL||msgId==NULL){
			cout<<"Empty msg to add?"<<endl;
			evbuffer_free(buf);
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
		evbuffer_add_printf(buf, "%s", "Delete Queue Request?Roger that");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else if(strcmp(url_path,DELETE_MSG.c_str())==0){
		//TODO:delete message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName and delete a message? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			return;
		}
		const char* msgId = evhttp_find_header(url_parameters,MSG_ID.c_str());
		if(msgId==NULL){
			cout<<"Empty msgId to delete?"<<endl;
			evbuffer_free(buf);
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
		evbuffer_add_printf(buf, "%s", "Delete Msg order is in action!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;

	}else if(strcmp(url_path,GET_MSG.c_str())==0){
		//TODO:get message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName and get a message? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
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
		return;
	}else if(strcmp(url_path,RECOVERY.c_str())==0){ /* recovery*/
		const char* logsize = evhttp_find_header(url_parameters,LOG_SIZE.c_str());
		int sz = logger->count()-atoi(logsize);
		vector<string> datas =  logger->tailList(sz);
		for(vector<string>::iterator iter = datas.begin();iter!=datas.end();iter++){
			dispatchMessage(nodeName,atoi(nodePort),*iter);
		}
	}else if(strcmp(url_path,JOIN_TEAM.c_str())==0){/*  join*/
		const char* publicNodeName = evhttp_find_header(url_parameters,PUBLIC_NODE_NAME.c_str());
		const char* publicNodePort = evhttp_find_header(url_parameters,PUBLIC_NODE_PORT.c_str());
		if(publicNodeName==NULL||publicNodePort==NULL){
			evbuffer_add_printf(buf, "%s", "public node name or port is NULL?");
			evhttp_send_reply(req, HTTP_OK, "NO", buf);
			evbuffer_free(buf);
			return;
		}
		DataNode* dNode = new DataNode(publicNodeName,atoi(publicNodePort),nodeName,atoi(nodePort),10000);
		mDataNodes.push_back(*dNode);
		pBeat->AddDataNode(*dNode);
		evbuffer_add_printf(buf, "%s", "Welcome to join us!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else if(strcmp(url_path,EMPTY_PATH.c_str())==0){
		//TODO:other opt
		evbuffer_add_printf(buf, "%s", "No other opts, sorry, my dear hacker!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else{
		//TODO: hacker?
		evbuffer_add_printf(buf, "%s", "What is it?my dear hacker!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;

	}
}

void SQSMaster::onClientReqRecv (struct evhttp_request* req) {
	cout<<"Receive msg from client."<<req->uri<<endl;
    struct evbuffer *buf;
    buf = evbuffer_new();

    const char *url_path;
    url_path =evhttp_uri_get_path(req->uri_elems);
    //cout<<":"<<req->uri<<":"<<httpsqs_query_part<<endl;
	if(strcmp(url_path,GET_AVAILABLE_HOST.c_str())==0){
		cout<<"Client request recv..."<<endl;
		//TODO: get one available host and send back.
		evbuffer_add_printf(buf, "%s", "Correct request......");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
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
