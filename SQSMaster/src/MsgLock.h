/**
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


/**
 *  timeout event base class
 *
 */
class Event
{
public:
	/**
	 * call back function for event process. If the event is processed, it should also be deleted from
	 * the event chain
	 */
	typedef void (*do_schedule)(void* arg,std::string name,int id);
	/**
	 * scheduled time in milliseconds
	 */
	long sched_time;
	 /**
	  * real time when the event is added to the event chain in milliseconds
	  */
	long join_time;
	 /**
	  * next event in the chain
	  */
	class Event *next;
	/**
	 * call back function to deal with the event
	 */
	do_schedule customSchedule;
	/* call back parameters */
	/**
	 * the context
	 */
	void* arg;
	/**
	 * the queue name
	 */
	std::string queue;
	/**
	 * the message id
	 */
	int msgId;
public:
	Event() { next = NULL;sched_time = 10;join_time=0;customSchedule=NULL;queue="EMPTY",msgId=-1;}
	Event(long sched_times,do_schedule sche,void* ar,std::string qu,int id):sched_time(sched_times),customSchedule(sche)
		,arg(ar),queue(qu),msgId(id){}
	/**
	 * @brief for test only
	 */
	Event(long sched_times,do_schedule sche=NULL):sched_time(sched_times),customSchedule(sche){}

	bool operator==(Event& instance){
		return instance.msgId==msgId&&instance.queue.compare(queue)==0&&instance.sched_time==sched_time;
	}
};
/**
 *  timeout chain class -
 *   the timeout process core
 */
class EventChain{
private:
	/**
	 * head event in the chain
	 */
	Event *head;
	/**
	 * the thread id the do the event process
	 */
	pthread_t pid;
	/**
	 * condition var to avoid overhead loop
	 */
	pthread_cond_t qready;
	/**
	 *  mutex to avoid racing
	 */
	pthread_mutex_t qlock;
	/**
	 * helper method to deal with dead lock. It just unlock the mutex
	 */
	friend void Cleanup(void* arg);
public:
	EventChain():head(NULL),pid(0),qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER){
	}
	virtual ~EventChain(){
		if(head){
			while(head->next!=NULL){
				Event* e = head;
				head = head->next;
				delete e;
			}
			delete head ;
		}
	}

	/**
	 * start handling event chain, please do not call it directly, call Start() instead
	 */
	void Run();
	/**
	 * call it to start process event chain
	 *
	 * @returns true if successfully start otherwise false
	 */
	bool Start();
	/**
	 * call it to stop process event chain. Please keep it in mind that you call this method
	 * only on condition you have to or on terminate for it will not only stop the event chain,
	 * but also free the chain list.
	 *
	 * @returns true if successfully stop otherwise false
	  */
	bool Stop();
	/**
	 * schedule a new event
	 *
	 * @param e the event to be scheduled
	 */
	void Schedule(Event* e);
	/**
	 *
	 * cancel an event
	 *
	 *@param e the event to be canceled
	 *
	 * if event not found, just free the parameter e
	 * else free both the parameter e and the matching event
	 */
	void Cancel(Event* e);
};

/**
 * @brief
 * class to handle message lock and unlock.
 * It can handle time out event automatically
 */
class MsgLock{
private:
	EventChain* chain_core;
	/**
	 * locked messages,i.e.{queue name: { id1,id2,id3}}
	 */
    std::multimap<std::string,int> mDatas;
    /**
     * default time out for the message lock, 5s in default
     */
    long defaultTmout;
	pthread_cond_t qready;
	pthread_mutex_t qlock;
	/**
	 * @brief
	 * Whether the giving msg id in the giving queue is already lock
	 * returns true if already lock
	 */
    bool IsExist(std::string queueName,int msgId);
    /**
     * @brief
     * insert the message to the lock list.
     * It will always return true
     */
    bool Insert(std::string queueName,int msgId);
    /**
     * @brief
     * Delete the locked message
     * lock it when processing
     * @param queueName	the queue name
     * @param msgId the message id to be unlocked
     * returns true if successfully unlock, false if message could not be found and unlocked
     */
    bool Del(std::string queueName,int msgId);
public:
	MsgLock():defaultTmout(5000),chain_core(new EventChain()),qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER){ }
	MsgLock(long tmout):defaultTmout(tmout),chain_core(new EventChain()),qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER){}
	/**
	 * @brief lock the message
	 * returns true if successfully lock, false if the message is already locked by others
	 */
	bool Lock(std::string queueName,int msgId);//should lock it when processing
	/**
	 * @brief unlock the message
	 * this will always return true
	 */
	bool UnLock(std::string queueName,int msgId);
	/**
	 * @brief start the event chain process.
	 * this is for the lock time out handling
	 */
	bool Start();
	/**
	 * @brief stop the event chain process.
	 * remember to call it on terminate and only once!
	 */
	bool Stop();

	/**
	 * @brief
	 * call back function to do with the event process: unlock the message and delete the event from
	 * the event list
	 *
	 * @param arg the context environment, that's, the Msglock instance
	 * @param name the queue name
	 * @param id the message id
	 */
	friend void ReleaseLock(void* arg,std::string name,int id);
};

#endif
