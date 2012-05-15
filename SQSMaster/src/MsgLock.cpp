/*
 * MsgLock.cpp
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 */

#include "MsgLock.h"

void EventChain::Cancel(Event* e ){

}
void EventChain::Schedule(Event* e){

}

void EventChain::Run(){

}



bool MsgLock::Lock(std::string queueName,int msgId){
	return false;
}
bool MsgLock::UnLock(std::string queueName,int msgId){
	return false;
}
