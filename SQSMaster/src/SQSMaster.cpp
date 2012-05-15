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
	cout<<"Receive msg from data node "<<req->uri<<endl;
	cout<<clientPort<<":"<<dataNodePort<<endl;
}

void SQSMaster::onClientReqRecv (struct evhttp_request* req) {
	cout<<"Receive msg from client"<<req->uri<<endl;
	cout<<clientPort<<":"<<dataNodePort<<endl;

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
	event_dispatch();
}
