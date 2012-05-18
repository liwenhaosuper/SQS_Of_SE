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

#include "Convention.h"
#include <string.h>
using namespace std;

void ClientCallBack(struct evhttp_request* req, void* arg){
	SQSMaster* obj = (SQSMaster*)arg;
	obj->onClientReqRecv(req);
}

void DataNodeCallBack(struct evhttp_request* req, void* arg){
	SQSMaster* obj = (SQSMaster*)arg;
	obj->onDataNodeRecv(req);
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

	if(strcmp(url_path,CREATE_QUEUE.c_str())==0){
		//TODO: create queue
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName to create? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			return;
		}
	}else if(strcmp(url_path,DEL_QUEUE.c_str())==0){
		//TODO: delete queue
		const char* queueName = evhttp_find_header(url_parameters,QUEUE_NAME.c_str());
		if(queueName==NULL){
			evbuffer_add_printf(buf, "%s", "Empty queueName to delete? Are you joking?");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			return;
		}
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
	}else if(strcmp(url_path,DELETE_MSG.c_str())==0){
		//TODO:delete message
		evbuffer_add_printf(buf, "%s", "Delete Msg order is in action!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;

	}else if(strcmp(url_path,GET_MSG.c_str())==0){
		//TODO:get message
		evbuffer_add_printf(buf, "%s", "get Msg order is in action!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;

	}else if(strcmp(url_path,EMPTY_PATH.c_str())==0){
		//TODO:other opt
		evbuffer_add_printf(buf, "%s", "No other opts, sorry, my dear hacker!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		return;
	}else if(url_path,RECOVERY.c_str()==0){
		//TODO: recovery mode
		evbuffer_add_printf(buf, "%s", "Recovery mode is launching!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
	}else if(url_path,JOIN_TEAM.c_str()==0){
		//TODO: data node join
		evbuffer_add_printf(buf, "%s", "Welcome to join us!");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
	}
	else{
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
