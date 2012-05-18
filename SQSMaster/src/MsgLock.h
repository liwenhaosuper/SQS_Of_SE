/*
 * MsgLock.h
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 */
#ifndef MSGLOCK_H
#define MSGLOCK_H

#include <string>
#include <pthread.h>
#include <map>

/* timeout event base class */
class Event
{
public:
	typedef void (*do_schedule)(void* arg,std::string name,int id);
	long sched_time;      /* scheduled occuring time in mili-seconds*/
	long join_time;       /* real time when the event is added to the event chain in mili-seconds */
	class Event *next;      /* next event in the chain */
	do_schedule customSchedule;
	//call back parameters
	std::string queue;
	int msgId;
	void* arg;
public:
	Event() { next = NULL;sched_time = 10;join_time=0;customSchedule=NULL;queue="EMPTY",msgId=-1;}
	Event(long sched_times,do_schedule sche,void* ar,std::string qu,int id):sched_time(sched_times),customSchedule(sche)
		,arg(ar),queue(qu),msgId(id){}
	//for test only
	Event(long sched_times,do_schedule sche=NULL):sched_time(sched_times),customSchedule(sche){}
	bool operator==(Event& instance){
		return instance.msgId==msgId&&instance.queue.compare(queue)==0&&instance.sched_time==sched_time;
	}
};
/* timeout chain class - the timeout process core */
class EventChain{
private:
	Event *head;            /* head event in the chain */
	pthread_t pid;
	pthread_cond_t qready;
	pthread_mutex_t qlock;

public:
	EventChain():head(NULL),pid(0),qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER){
	}
	friend void Cleanup(void* arg);
	void Run(); /* start handling event chain, please do not call it directly, call Start() instead */
	bool Start();/* call it to start process event chain */
	bool Stop(); /* call it to stop process event chain. Please keep it in mind that you call this method only on condition you have to. */
	void Schedule(Event* e);
	void Cancel(Event* e);
};

class MsgLock{
private:
	EventChain* chain_core;
    std::multimap<std::string,int> mDatas;//queue name: { id1,id2,id3}
    long defaultTmout;// 5s in default
	pthread_cond_t qready;
	pthread_mutex_t qlock;
    bool IsExist(std::string queueName,int msgId);
    bool Insert(std::string queueName,int msgId);
    bool Del(std::string queueName,int msgId);// should lock it when processing
public:
	MsgLock():defaultTmout(5000),chain_core(new EventChain()),qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER){ }
	MsgLock(long tmout):defaultTmout(tmout),chain_core(new EventChain()),qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER){}
	bool Lock(std::string queueName,int msgId);//should lock it when processing
	bool UnLock(std::string queueName,int msgId);
	bool Start();
	bool Stop();
	friend void ReleaseLock(void* arg,std::string name,int id);
};

#endif
