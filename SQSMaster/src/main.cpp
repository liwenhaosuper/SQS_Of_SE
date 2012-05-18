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
