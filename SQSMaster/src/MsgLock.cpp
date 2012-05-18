/*
 * MsgLock.cpp
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 */

#include "MsgLock.h"
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <map>

using namespace std;

void* startThread(void* arg){
	if(arg==NULL){
		return (void*)0;
	}else{
		EventChain* chain = (EventChain*) arg;
		chain->Run();
		return (void*)0;
	}
}


void EventChain::Cancel(Event* e ){
	if(head==NULL||e==NULL){
		return;
	}
	//cout<<"Canceling..."<<endl;
	pthread_mutex_lock(&qlock);//cout<<"EventChain::Cancel.lock."<<endl;
	Event* pCur = head;
	if((*pCur)==(*e)){
		head = head->next;
		delete pCur;
		pthread_mutex_unlock(&qlock);//cout<<"EventChain::Cancel.unlock."<<endl;
		return;
	}
	while(pCur->next!=NULL){
		if((*pCur->next)==(*e)){
			pCur->next = pCur->next->next;
			delete e;
			pthread_mutex_unlock(&qlock);//cout<<"EventChain::Cancel.unlock."<<endl;
			return;
		}else{
			pCur = pCur->next;
		}
	}
	pthread_mutex_unlock(&qlock);//cout<<"EventChain::Cancel.unlock."<<endl;
}

bool EventChain::Start(){
	int err;
	err = pthread_create(&pid,NULL,startThread,this);
    if(err!=0){
    	//cout<<"Fail to start thread.Error msg:can't create	thread:"<<strerror(err)<<std::endl;
    	return false;
    }
	return true;
}
bool EventChain::Stop(){
	if(pthread_cancel(pid)==0){
		if(head ==NULL){
			return true;
		}
		while(head->next==NULL){
			Event* e = head;
			head = head->next;
			delete e;
		}
		head = NULL;
		return true;
	}
	return false;
}

void EventChain::Schedule(Event* e){
	if(e==NULL){
		return;
	}
	pthread_mutex_lock(&qlock);//cout<<"EventChain::Schedule.lock."<<endl;
	time_t curtime;
	time(&curtime);
	e->join_time = 1000*((long)curtime);
	if(head==NULL){
		head = e;
		head->next = NULL;
		pthread_mutex_unlock(&qlock);//cout<<"EventChain::Schedule.unlock."<<endl;
		pthread_cond_signal(&qready);
	}else if(head->sched_time-((1000*curtime)-head->join_time)>e->sched_time){
		Event* p = head;
		head = e;
		head->next = p;
		pthread_mutex_unlock(&qlock);//cout<<"EventChain::Schedule.unlock."<<endl;
		pthread_cond_signal(&qready);
	}else{
		Event* pCur = head;
		while(pCur->next!=NULL){
			if(pCur->next->sched_time-((1000*curtime)-pCur->next->join_time)>e->sched_time){
				Event* tmp = pCur->next;
				pCur->next = e;
				e->next = tmp;
				pthread_mutex_unlock(&qlock);//cout<<"EventChain::Schedule.unlock."<<endl;
				return;
			}
			pCur = pCur->next;
		}
        pCur->next = e;
        e->next = NULL;
    	pthread_mutex_unlock(&qlock);//cout<<"EventChain::Schedule.unlock."<<endl;
	}
}

void Cleanup(void* e){
     if(e!=NULL){
    	 EventChain* chain = (EventChain*) e;
    	 pthread_mutex_unlock(&chain->qlock);//cout<<"Cleanup.unlock"<<endl;
     }
}

void EventChain::Run(){
	 struct timeval now;
	 struct timespec tsp;
	 pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
	 pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);
     while(true){
    	 while(head!=NULL){
    		pthread_testcancel();
    		pthread_cleanup_push(Cleanup,this);
    		pthread_mutex_lock(&qlock);//cout<<"EventChain::Run.lock0."<<endl;
    		time_t curtime;
    		time(&curtime);
    		long left = (head->sched_time-((1000*curtime)-head->join_time));
    		while(left>0){
    			gettimeofday(&now,NULL);
    			tsp.tv_sec = now.tv_sec+left/1000;
    			tsp.tv_nsec = now.tv_usec*1000+left%1000;
    			pthread_cond_timedwait(&qready,&qlock,&tsp);
    			time(&curtime);
    			left = (head->sched_time-((1000*curtime)-head->join_time));
    		}
    		do{
    			pthread_mutex_unlock(&qlock);//cout<<"EventChain::Run.unlock."<<endl;
    			head->customSchedule(head->arg,head->queue,head->msgId);
    			if(head==NULL){
    				break;
    			}
    			pthread_mutex_lock(&qlock);//cout<<"EventChain::Run.lock."<<endl;
    			time(&curtime);
    			left = (head->sched_time-((1000*curtime)-head->join_time));
    		}while(left<=0);
    		pthread_mutex_unlock(&qlock);//cout<<"EventChain::Run.unlock."<<endl;
    		pthread_cleanup_pop(0);
    	 }
    	 pthread_cleanup_push(Cleanup,this);
    	 pthread_mutex_lock(&qlock);//cout<<"EventChain::Run.lock2"<<endl;
    	 while(head==NULL){
    		 pthread_testcancel();
    		 pthread_cond_wait(&qready,&qlock);
    	 }
    	 pthread_mutex_unlock(&qlock);//cout<<"EventChain::Run.unlock2."<<endl;
    	 pthread_cleanup_pop(0);
     }
}



bool MsgLock::Lock(std::string queueName,int msgId){
	bool res = false;
	pthread_mutex_lock(&qlock);//cout<<"MsgLock::Lock.lock."<<endl;
	if(IsExist(queueName,msgId)){
		res = false;
	}else{
		res = Insert(queueName,msgId);
	}
	pthread_mutex_unlock(&qlock);//cout<<"MsgLock::Lock.unlock."<<endl;
	return res;
}
bool MsgLock::UnLock(std::string queueName,int msgId){
	Del(queueName,msgId);
	return true;
}
bool MsgLock::IsExist(std::string queueName,int msgId){
	multimap<std::string,int>::iterator iter = mDatas.find(queueName);
	while(iter!=mDatas.end()){
		if(iter->second==msgId){
			return true;
		}
		iter++;
	}
	return false;
}
void ReleaseLock(void* arg,std::string name,int id){
	//cout<<"ReleaseLock..."<<endl;
	if(arg==NULL){
		//cout<<"Schedule:arg is null..."<<endl;
		return;
	}
	MsgLock* lock = (MsgLock*) arg;
    lock->Del(name,id);
    //cout<<"ReleaseLocked..."<<endl;
}
bool MsgLock::Insert(std::string queueName,int msgId){
	mDatas.insert(make_pair(queueName,msgId));
	Event* e = new Event(defaultTmout,ReleaseLock,this,queueName,msgId);
	chain_core->Schedule(e);
    return true;
}
bool MsgLock::Del(std::string queueName,int msgId){
	pthread_mutex_lock(&qlock);//cout<<"MsgLock::Del.lock."<<endl;
	Event* e = new Event(defaultTmout,ReleaseLock,this,queueName,msgId);
	chain_core->Cancel(e);
	multimap<std::string,int>::iterator iter = mDatas.find(queueName);
	while(iter!=mDatas.end()){
		if(iter->second==msgId){
			mDatas.erase(iter++);
			pthread_mutex_unlock(&qlock);//cout<<"MsgLock::Del.unlock."<<endl;
			return true;
		}
		iter++;
	}
	pthread_mutex_unlock(&qlock);//cout<<"MsgLock::Del.unlock."<<endl;
	return false;
}

bool MsgLock::Start(){
	return chain_core->Start();
}
bool MsgLock::Stop(){
	return chain_core->Stop();
}
