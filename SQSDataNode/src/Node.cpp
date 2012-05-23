#include "Node.h"
#include <iostream>
#include <err.h>
#include "Logger.h"
#include "event.h"
#include "evhttp.h"
#include "event2/http.h"
#include <sys/time.h>
#include "Convention.h"
#include <string.h>


#include <cstdlib>
using namespace std;

void ClientCallBackNode(struct evhttp_request* req, void* arg){
	Node* obj = (Node*)arg;
	obj->onClientRecv(req);
}
void MasterCallBack(struct evhttp_request* req, void* arg){
	Node* obj = (Node*)arg;
	obj->onMasterRecv(req);
}

bool Node::init(){
	event_init();
	struct evhttp *httpd_client,*httpd_master;
	httpd_master = evhttp_start("0.0.0.0",master_port);
	httpd_client = evhttp_start("0.0.0.0",client_port);
	if(httpd_client==NULL){
			fprintf(stderr,"ERROR:Unable to listen on %s:%d\n","0.0.0.0",client_port);
			return false;
	}
	if(httpd_master==NULL){
		fprintf(stderr,"ERROR:Unable to listen on %s:%d\n","0.0.0.0",master_port);
		return false;
	}
	evhttp_set_gencb(httpd_client, ClientCallBackNode, this);
	evhttp_set_gencb(httpd_master, MasterCallBack,this);
	Recovery();
	Join();
	event_dispatch();
	return true;
}

void recovery_callback(struct evhttp_request *req, void *rsp){

}
void Node::dispatch(string request){
	struct event_base *base = event_base_new();
	struct evhttp_connection *cn = evhttp_connection_base_new(
				base, NULL,
				masterAddress.c_str(),
				master_port);
	struct evhttp_request *req = evhttp_request_new(recovery_callback, base);
	if(evhttp_make_request(cn,req,EVHTTP_REQ_GET,request.c_str())==-1){
		cout<<"Make request fail..."<<endl;
	}
	event_base_dispatch(base);
}


bool Node::Recovery(){
	struct event_base *base = event_base_new();
	struct evhttp_connection *cn = evhttp_connection_base_new(
	        base, NULL,
	        masterAddress.c_str(),
	        master_port);
	struct evhttp_request *rsp = NULL;
	/* Allocate request structure */
	if ((rsp = (struct evhttp_request *)malloc(sizeof(struct evhttp_request))) == NULL) {
		cout<<"Error allocating rsp structure"<<endl;
		return false;
	}
	//cout<<"original rsp:"<<rsp<<endl;
	struct evhttp_request *req = evhttp_request_new(recovery_callback,rsp);
	evhttp_make_request(cn,req,EVHTTP_REQ_GET,RECOVERY.c_str());
	event_base_dispatch(base);
//	cout<<"sent heart beat:"<<rsp<<endl;
//	cout<<"rsp::::rsp body size:"<<((struct evhttp_request *)rsp)->body_size
//			<<":rsp code line:"<<((struct evhttp_request *)rsp)->response_code_line
//			<<":rsp header size:"<<((struct evhttp_request *)rsp)->headers_size<<endl;
	return true;
}
bool Node::Join(){
	return true;
}

void Node::onMasterRecv(struct evhttp_request *req){
	cout<<"Receive msg from master.path:"<<req->uri<<endl;
    struct evbuffer *buf;
    buf = evbuffer_new();
    struct evkeyvalq *url_parameters = new evkeyvalq;
    const char *url_path;
    url_path =evhttp_uri_get_path(req->uri_elems);
	if(strcmp(url_path,HEARTBEAT.c_str())==0){
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
	}else if(strcmp(url_path,RECOVERY.c_str())==0){
		const char* log = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(log!=NULL){
			logger = new Logger(log);
			if(db->init(log)){
				evbuffer_add_printf(buf, "%s", "Succeed");
				evhttp_send_reply(req, HTTP_OK, "OK", buf);
				delete log;
			}else{
				evbuffer_add_printf(buf, "%s", "Fail");
				evhttp_send_reply(req, HTTP_OK, "OK", buf);
			}
			delete url_parameters;
		}
	}else{
		evbuffer_add_printf(buf, "%s", "Unrecognized request......");
		evhttp_send_reply(req, HTTP_BADREQUEST, "NO", buf);
	}
	delete url_path;
	evbuffer_free(buf);
}
void Node::onClientRecv(struct evhttp_request *req){
	cout<<"Receive msg from client.path:"<<req->uri<<endl;
	struct evbuffer *buf;
	buf = evbuffer_new();
	struct evkeyvalq *url_parameters = new evkeyvalq;
	const char *url_path;
	url_path =evhttp_uri_get_path(req->uri_elems);
	if(strcmp(url_path,GET_MSG.c_str())==0){
		//get message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		const char* msgID = evhttp_find_header(url_parameters,MSG_ID.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/getMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			delete url_parameters;
			delete queueName;
			evbuffer_free(buf);
			return;
		}
		const char* content = db->getMessage(queueName,msgID).c_str();
		if(content!=NULL){
			string command = GET_MSG;
			command +="?"+QUEUE_NAME+"="+queueName;
			command +="?"+MSG_ID+"="+msgID;
			logger->addLog(command);
			evbuffer_add_printf(buf,"%s",content);
		}else{
			evbuffer_add_printf(buf, "%s", "Fail");
		}
		//how to send msg to master??
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		delete url_parameters;
		delete queueName;
		return;
	}else if(strcmp(url_path,PUT_MSG.c_str())==0){
		//put message
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "The request url should be this kind:http:hostName:hostPort/getMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=val5");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			delete url_parameters;
			delete queueName;
			evbuffer_free(buf);
			return;
		}
		string msg = evhttp_find_header(url_parameters,MSG.c_str());
		if(db->putMessage(msg)){
			string command = PUT_MSG;
			command +="?"+QUEUE_NAME+"="+queueName;
			logger->addLog(command);
			evbuffer_add_printf(buf, "%s", "Succeed");
		}else{
			evbuffer_add_printf(buf, "%s", "Fail");
		}
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		delete url_parameters;
		delete queueName;
		return;
	}else{
		evbuffer_add_printf(buf, "%s", "Unrecognized request......");
		evhttp_send_reply(req, HTTP_BADREQUEST, "NO", buf);
		delete url_parameters;
	}
	evbuffer_free(buf);

}
