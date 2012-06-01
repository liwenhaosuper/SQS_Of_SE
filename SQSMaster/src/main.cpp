//============================================================================
// Name        : SQSMaster.cpp
// Author      : liwenhaosuper
// Version     :
// Copyright   : No rights reserved
// Description : SQS in C++, Ansi-style
//============================================================================

#include <iostream>
#include <getopt.h>
#include <err.h>
#include "event.h"
#include "evhttp.h"
#include "SQSMaster.h"
#include <string>
#include <cstdlib>


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
//	        "127.0.0.1",
//	        8080);
//	struct evhttp_request *req = evhttp_request_new(req_callback, base);
//	evhttp_make_request(cn,req,EVHTTP_REQ_GET,"/Album");
//	event_base_dispatch(base);
//	cout<<"Bye"<<endl;
//}
//int main(){
//	cout<<"Testing heart beat socket..."<<endl;
//	create();
//	return 0;
//}

static struct option long_options[] = {
	{"clientPort",     required_argument, 0, 0},
	{"datanodePort",   required_argument, 0, 0},
	{"timeout",        required_argument, 0, 0},
	{"msgTimeout",     required_argument, 0, 0},
	{0,                0,                 0, 0}
};

void showUsage()
{
	const char *b = "-------------------------------------------------------------------------------\n"
			"--clientPort     <port>  port for client\n"
			"--datanodePort   <port>  port for datanode\n"
			"--timeout        <num>   keep-alive timeout for an http request (default: 6)\n"
			"--msgTimeout     <num>   keep-alive timeout for an http request (default: 5000)\n"
			"-h                       print this help and exit\n"
			"-------------------------------------------------------------------------------\n";
	printf("%s", b);
}

/**
 *  start service
 */
int main(int argc, char *argv[]) {
	int portPublic = 1200;
	int portDatanode = 1300;
	int timeout = 6;
	int msgTimeout = 5000;

	int c;

	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "h", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 0:
			switch (option_index) {
			case 0:
				portPublic = atoi(optarg);
				break;
			case 1:
				portDatanode = atoi(optarg);
				break;
			case 2:
				timeout = atoi(optarg);
				break;
			case 3:
				msgTimeout = atoi(optarg);
				break;
			default:
				break;
			}
			break;
		case 'h':
		default:
			showUsage();
			exit(1);
			break;
		}
	}
	SQSMaster* master = new SQSMaster(portPublic,portDatanode,timeout,msgTimeout);
	if(master->init()){
		cout << "<<<<<<<<<SQSMaster is started>>>>>>>>>>" << endl;
		master->start();
	}else{
		cout<<"Fail to start service. Bye"<<endl;
	}
}

/**
 * Test for getting client's request
 *
 * Get request from client:
 * case 1.
 * 			http://localhost:1200/getavailablehost
 * return:	No Data Node available...
 *
 * case 2.
 * 			http://localhost:1200/ahah
 * return:	Unrecognized request......
 *
 *
 *Get request from data node:
 *case 1.
 *			http://localhost:1300/createQueue?nodeName=node1&nodePort=10086&queueName=hi
 * return:  Create Queue Request?Roger that
 *
 *case 2.
 *			http://localhost:1300/recovery?nodeName=node1&nodePort=1100&logSize=1000
 *return:	Recovery is completed!
 *
 *case 3.
 *			http://localhost:1300/join?nodeName=node2&nodePort=1100&publicNodeName=master&publicNodePort=23
 *return:	Welcome to join us!
 *
 *case 4.
 *			http://localhost:1300/deleteQueue?nodeName=node1&nodePort=1111&queueName=queue
 *return && FIXME:   crash when trying to send message to an invalid node.
 *			http://localhost:1300/deleteQueue?nodeName=node1&nodePort=1111&queueName=queue
 *return:	Delete Queue Request?Roger that
 *FIX	:	Fixed!
 *
 *case 5.
 *			http://localhost:1300/join?nodeName=localhost&nodePort=1200&publicNodeName=master&publicNodePort=23
 *			http://localhost:1300/deleteQueue?nodeName=node1&nodePort=1111&queueName=queue
 *return:&& FIXME: crash when sending http request
 *FIX	:	Fixed!
 *
 *
 *
 */
