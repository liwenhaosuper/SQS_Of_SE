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

class SQSMaster{
private:
	int clientPort;
	int dataNodePort;
	int connectionTimeout;
	MsgLock* pLock;
public:
	SQSMaster():clientPort(1200),dataNodePort(1300),connectionTimeout(6){}
	SQSMaster(int client,int dataNode,int connTimeout):clientPort(client),dataNodePort(dataNode),connectionTimeout(connTimeout){}
	virtual ~SQSMaster(){if(pLock) delete pLock;};
	bool init();
	void start();
	void onDataNodeRecv (struct evhttp_request *req);/* invoke when a request from data node recv */
	void onClientReqRecv (struct evhttp_request *req);/* invoke when a request from client recv */
};


#endif /* SQSMASTER_H_ */
