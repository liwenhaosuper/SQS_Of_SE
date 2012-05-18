/*
 * SQSMaster.h
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 */

#ifndef SQSMASTER_H_
#define SQSMASTER_H_

#include <err.h>
#include "event.h"
#include "evhttp.h"
#include <string>

#include "MsgLock.h"

class DataNode{
private:
	std::string nodeName_public;
	int nodePort_public;
	bool alive;
	std::string nodeName_formaster;
	int nodePort_formaster;
public:
	DataNode(std::string nm,int port,std::string masterNM,int masterPort,bool live=false):
		nodeName_public(nm),nodePort_public(port),nodeName_formaster(masterNM),nodePort_formaster(masterPort),alive(live){}
};

class SQSMaster{
private:
	int clientPort;
	int dataNodePort;
	int connectionTimeout;
	MsgLock* pLock;
	long defaultTmout;
public:
	SQSMaster():clientPort(1200),dataNodePort(1300),connectionTimeout(6),pLock(new MsgLock(5000)){}
	SQSMaster(int client,int dataNode,int connTimeout,int msgTimeout):clientPort(client),dataNodePort(dataNode),connectionTimeout(connTimeout)
		,pLock(new MsgLock(msgTimeout)){}
	virtual ~SQSMaster(){if(pLock) delete pLock;};
	bool init();
	void start();
	void onDataNodeRecv (struct evhttp_request *req);/* invoke when a request from data node recv */
	void onClientReqRecv (struct evhttp_request *req);/* invoke when a request from client recv */
};


#endif /* SQSMASTER_H_ */
