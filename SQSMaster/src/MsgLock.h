/*
 * MsgLock.h
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 */
#ifndef MSGLOCK_H
#define MSGLOCK_H

#include <string>

/* timeout event base class */
class Event
{
public:
	double sched_time;      /* scheduled occuring time */
	class Event *next;      /* next event in the chain */

public:
	Event() { next = NULL; }
};
/* timeout chain class - the timeout process core */
class EventChain{
private:
	double timeOut;
	Event *head;            /* head event in the chain */
public:
	EventChain();
	void Run(); /* start handling event chain */
	void Schedule(Event* e);
	void Cancel(Event* e);
};

class MsgLock{
private:
	EventChain* chain_core;
public:
	MsgLock(){};
	MsgLock(double defaultTimeout){};
	bool Lock(std::string queueName,int msgId);
	bool UnLock(std::string queueName,int msgId);
};

#endif
