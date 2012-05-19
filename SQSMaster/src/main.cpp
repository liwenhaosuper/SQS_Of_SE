//============================================================================
// Name        : SQSMaster.cpp
// Author      : liwenhaosuper
// Version     :
// Copyright   : No rights reserved
// Description : SQS in C++, Ansi-style
//============================================================================

#include <iostream>
#include <err.h>
#include "event.h"
#include "evhttp.h"
#include "SQSMaster.h"
#include <string>


#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/http_struct.h>

using namespace std;

void callback(void* arg,string name="",int msgId=2){
	cout<<"hello world..."<<endl;
}




   /* test event chain */
//int main() {
//	cout << "Testing event chain......" << endl;
//	EventChain* chain = new EventChain;
//	chain->Start();
//	for(int i=0;i<50;i++){
//		Event* e = new Event;
//		e->customSchedule = callback;
//		e->sched_time = 300*i+1;
//		chain->Schedule(e);
//		//cout << "SQSMaster is onProcess." << endl;
//	}
//	for(int i=0;i<50;i++){
//		Event* e = new Event;
//		e->customSchedule = callback;
//		e->sched_time = 10*i+1;
//		chain->Schedule(e);
//		//cout << "SQSMaster is onProcess." << endl;
//	}
//	sleep(2);
//	chain->Stop();
//	sleep(50);
//	cout << "SQSMaster is ended." << endl;
//	return 0;
//}

//operator == test
//int main(){
//	Event* e = new Event(1000,NULL,NULL,"hi",1);
//	Event* e1 = new Event(1000,NULL,NULL,"hi",1);
//	e1->next = new Event(1000,NULL,NULL,"hi",1);
//	if(e==e1){
//		cout<<"1"<<endl;
//	}
//	if((*e)==(*e1->next)){
//		cout<<"2"<<endl;
//	}
//	cout<<"End"<<endl;
//}

//test msglock
//int main(){
//	MsgLock* lock = new MsgLock(100);
//	lock->Start();
//	cout<<lock->Lock("hi",1)<<endl;
//	cout<<lock->Lock("hi",2)<<endl;
//	cout<<lock->Lock("hi",3)<<endl;
//	sleep(1);
//	cout<<"!!!Wakeup.."<<endl;
//	cout<<lock->Lock("hi",1)<<endl;
//	cout<<lock->Lock("hi",1)<<endl;
//	cout<<lock->Lock("hi",2)<<endl;
//	cout<<lock->Lock("hi",3)<<endl;
//}

//test heart beat
//int main(){
//	cout<<"...Testing heart beat..."<<endl;
//	HeartBeat* beat = new HeartBeat(1000);
//	DataNode* node1 = new DataNode("0.0.0.0",1400,"1.1.1.1",1500,5000);
//	beat->AddDataNode(*node1);
//	sleep(3);
//	beat->CancelDataNode(*node1);
//	sleep(60);
//	return 0;
//}


//void req_callback(struct evhttp_request *req, void *arg){
//	struct evbuffer *buffer = evbuffer_new();
//	evbuffer_add_buffer(buffer, req->input_buffer);
//	cout<<"Request recv..."<<evbuffer_get_length(req->output_buffer)<<endl;
//	cout<<req->uri<<":"<<req->body_size<<":"<<req->response_code<<endl;
//}
//void create(){
//	struct event_base *base = event_base_new();
//	struct evhttp_connection *cn = evhttp_connection_base_new(
//	        base, NULL,
//	        "localhost",
//	        1300);
//	struct evhttp_request *req = evhttp_request_new(req_callback, NULL);
//	evhttp_make_request(cn,req,EVHTTP_REQ_GET,"/join");
//	event_base_dispatch(base);
//	cout<<"Bye"<<endl;
//}
//int main(){
//	cout<<"Testing heart beat socket..."<<endl;
//	create();
//	return 0;
//}


/* start service */
int main() {
	SQSMaster* master = new SQSMaster(1200,1300,5,5000);
	if(master->init()){
		cout << "SQSMaster is started." << endl;
		master->start();
	}else{
		cout<<"Fail to start service. Bye"<<endl;
	}
}
