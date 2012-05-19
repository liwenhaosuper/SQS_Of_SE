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
#include <vector>
#include "MsgLock.h"

class DataNode{
private:
	std::string nodeName_public;
	int nodePort_public;
	std::string nodeName_formaster;
	int nodePort_formaster;
	long defaultTimeout;//in mill-seconds
	long startTime;
	bool cancel;
public:
	DataNode(std::string nm,int port,std::string masterNM,int masterPort,long timeout):cancel(false),
		nodeName_public(nm),nodePort_public(port),nodeName_formaster(masterNM),nodePort_formaster(masterPort),defaultTimeout(timeout){}
	void Recycle();
	bool IsAlive();
	bool IsCancel(){return cancel;}
	void Cancel(){ cancel = true; }
	DataNode(const DataNode& node){
		this->defaultTimeout = node.defaultTimeout;
		this->nodeName_formaster = node.nodeName_formaster;
		this->nodeName_public = node.nodeName_public;
		this->nodePort_formaster = node.nodePort_formaster;
		this->nodePort_public = node.nodePort_public;
		this->startTime = node.startTime;
		this->cancel = node.cancel;
	}
	bool operator==(DataNode& instance){
		return  instance.nodeName_formaster.compare(nodeName_formaster)==0&&
				instance.nodeName_public.compare(nodeName_public)==0&&
				instance.nodePort_formaster==nodePort_formaster&&
				instance.nodePort_public==nodePort_public;
	}
	std::string getNodeNameFormaster() const {
		return nodeName_formaster;
	}
	std::string getNodeNamePublic() const {
		return nodeName_public;
	}
	int getNodePortFormaster() const {
		return nodePort_formaster;
	}
	int getNodePortPublic() const {
		return nodePort_public;
	}
};


class HeartBeat{
private:
	long cycle;//in millisecond
	std::vector<DataNode*> clients;
	/* construct a simple socket and return its descriptor, return -1 if fails */
	struct evhttp_request *doRequest(std::string dataNode,int port);
public:
	HeartBeat(long scheTime):cycle(scheTime){}
	void AddDataNode(DataNode node);
	void CancelDataNode(DataNode node);
	bool IsNodeAlive(DataNode node);
	friend void* RegularCheck(void* arg);
};

class Param{
public:
	HeartBeat* instance;
	DataNode* node;
};

class SQSMaster{
private:
	int clientPort;
	int dataNodePort;
	int connectionTimeout;
	MsgLock* pLock;
	long defaultTmout;
	std::vector<DataNode> mDataNodes;
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
