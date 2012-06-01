/**
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
#include "Logger.h"

#include <deque>


#define MASTER_D 0

/**
 * @brief
 * DataNode
 * A data node object has two public ports, one for client communication, one for master communication
 */
class DataNode{
private:
	/**
	 * node name for client connection
	 */
	std::string nodeName_public;
	/**
	 *  node port for client connection
	 */
	int nodePort_public;
	/**
	 * node name for communication with the master
	 */
	std::string nodeName_formaster;
	/**
	 * node port for communication with the master
	 */
	int nodePort_formaster;
	/**
	 * default time out for the data node, in milliseconds
	 */
	long defaultTimeout;
	/**
	 * the time it joins the data node list, in milliseconds
	 */
	long startTime;
	/**
	 * whether it has been cancel or not.
	 */
	bool cancel;
public:
	DataNode(std::string nm,int port,std::string masterNM,int masterPort,long timeout):cancel(false),
		nodeName_public(nm),nodePort_public(port),nodeName_formaster(masterNM),nodePort_formaster(masterPort),defaultTimeout(timeout){}
	/**
	 * @brief refresh the startTime to the current time stamp
	 */
	void Recycle();
	/**
	 * @brief
	 * tell whether this data node is still alive or not
	 * A data node is alive only if it is not time out
	 */
	bool IsAlive();
	/**
	 * @brief whether it has been canceled or not
	 */
	bool IsCancel(){return cancel;}
	/**
	 * @brief cancel the data node
	 */
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

/**
 * @brief
 * heart beat handling class with the data nodes
 */
class HeartBeat{
private:
	/**
	 * the heart beat cycle in millisecond
	 */
	long cycle;
	/**
	 * data node list
	 */
	std::vector<DataNode*> clients;
	/**
	 * @brief
	 * send a simple http heart beat request to the giving data node and
	 * return its response. return NULL if fail.
	 * @warning It is your responsibility to free the memory
	 * */
	char *doRequest(std::string dataNode,int port);
public:
	HeartBeat(long scheTime):cycle(scheTime){}
	virtual ~HeartBeat(){
		for(std::vector<DataNode*>::iterator iter = clients.begin();iter!=clients.end();iter++){
			DataNode* node = *iter;
			delete node;
			*iter=NULL;
		}
		clients.clear();
	}
	/**
	 * add a data node the the data node list and start regular heart beat
	 */
	void AddDataNode(DataNode node);
	/**
	 * cancel the data node and remove it from the data node list
	 */
	void CancelDataNode(DataNode node);
	/**
	 * tell whether the data node is still alive
	 * if it is no longer alive, this method will remove it from the data node list
	 */
	bool IsNodeAlive(DataNode node);
	/**
	 * @brief do the regular check for a giving data node
	 * @param arg the Param object, containing the heart beat instance and the data node object to be processed
	 */
	friend void* RegularCheck(void* arg);
};
/**
 * @brief helper class to transfer parameters
 */
class Param{
public:
	HeartBeat* instance;
	DataNode* node;
};

/**
 * @brief helper structure for response call back
 */
struct RspParam{
    char* rsp;
    struct event_base* base;
    struct evhttp_connection *conn;
    struct evhttp_request *req;
};

/**
 * @brief SyncWorker is a thread worker that synchronize the data queue into log file
 *  interval
 */
class SyncWorker{
public:
	SyncWorker(long interval,std::deque<std::string>* p,Logger* log):
		sync_interval(interval),locks(0),stop(true),
		qready(PTHREAD_COND_INITIALIZER),qlock(PTHREAD_MUTEX_INITIALIZER),
	    pQueue(p),logger(log){}
	/**
	 * @brief call to start the SyncWorker
	 */
	bool Start();
	/**
	 * @brief stop the SyncWorker
	 */
	bool Stop();
	/**
	 * @brief add a lock to the worker
	 */
	void IncrementLock(std::string id);
	/**
	 * @brief remove a lock from the worker
	 */
	void DecrementLock(std::string id);

private:
	/**
	 * list of locks waiting for time out
	 */
	std::vector<std::string> locksVec;
	/**
	 *  logger for message processing
	 */
	Logger* logger;
	/**
	 * message queue
	 */
	std::deque<std::string>* pQueue;
	/**
	 * number of locks
	 */
	int locks;
	/**
	 *  synchronize interval in milliseconds
	 */
	long sync_interval;
	/**
	 *  flag to stop the worker
	 */
	volatile bool stop;

	pthread_cond_t qready;
	pthread_mutex_t qlock;
private:
	/**
	 * @brief internal start the worker
	 */
	void Run();
	friend void* lockCheckCallback(void* instance);
	friend void* startCallback(void* instance);
};


/**
 * @brief the main class for the master server
 */
class SQSMaster{
private:
	/**
	 * port number for the client connection
	 */
	int clientPort;
	/**
	 * port number for the data node connection
	 */
	int dataNodePort;
	/**
	 * default connection time out
	 */
	int connectionTimeout;
	/**
	 * lock for message
	 */
	MsgLock* pLock;
	/**
	 * all available data nodes
	 */
	std::vector<DataNode> mDataNodes;
	/**
	 *  logger for message processing
	 */
	Logger* logger;
	/**
	 * Heart beat checker
	 */
	HeartBeat* pBeat;
	/**
	 * disk syncworker
	 */
	SyncWorker* pSyncWorker;
	/**
	 * message queue
	 */
	std::deque<std::string>* pQueue;
public:
	SQSMaster():clientPort(1200),dataNodePort(1300),connectionTimeout(6),pLock(new MsgLock(5000)),
		logger(new Logger("SQS.log")),pBeat(new HeartBeat(3000)){
		pQueue = new std::deque<std::string>;
		pSyncWorker = new SyncWorker(1000,pQueue,logger);
	}
	SQSMaster(int client,int dataNode,int connTimeout,int msgTimeout):clientPort(client),dataNodePort(dataNode),connectionTimeout(connTimeout)
		,pLock(new MsgLock(msgTimeout)),logger(new Logger("SQS.log")),pBeat(new HeartBeat(3000))
	{
		pQueue = new std::deque<std::string>;
		pSyncWorker = new SyncWorker(1000,pQueue,logger);
	}

	/**
	 * @brief dispatch the giving request to the remote node.
	 * @param remoteNode the remote node name
	 * @param remotePort the remote port
	 * @param request the url path that containing all the datas, i.e. '/createQueue?queueName=queue1'
	 */
	void dispatchMessage(std::string remoteNode,int remotePort,std::string request);

	virtual ~SQSMaster(){
		if(pLock){
			pLock->Stop();
			delete pLock;
		}
	}

	/**
	 * do all the necessary things to start the master
	 * returns true if every goes fine, otherwise false
	 */
	bool init();
	/**
	 *start listening on master.
	 *Make sure you have called init() and init() returns true before
	 *you can call this method
	 */
	void start();
	/**
	 * @brief
	 * invoke when a request from data node received
	 * The request url should be this kind:
	 * 		http:hostName:hostPort/operation?param1=value1&param2=value2
	 *
	 *@warning All the requests should contain its node name and node port which is used to
	 *communicate with the master, i.e.
	 *		http:hostName:hostPort/operation?nodeName=value1&nodePort=value2.
	 *
	 *@warning The redirect message is the same with the log message
	 *
	 *@b 1
	 *<b>  To create a queue, the uri should be:
	 *		http:hostName:hostPort/createQueue?nodeName=value1&nodePort=value2&queueName=value3.
	 *The log and redirect message will be: /createQueue?queueName=value1
	 *If succeeds, the return message is "Create Queue Request Accepted!"
	 *</b>
	 *
	 *@b 2
	 *<b>  To delete a queue, the uri should be:
	 *		http:hostName:hostPort/deleteQueue?nodeName=value1&nodePort=value2&queueName=value3.
	 *The log and redirect message will be: /deleteQueue?queueName=value1
	 *If succeeds, the return message is "Delete Queue Request Accepted!"
	 *</b>
	 *
	 *@b 3
	 *<b>
	 *  To put a message, the url should be:
	 *		http:hostName:hostPort/putMessage?nodeName=value1&nodePort=value2&queueName=value3&message=value4&mId=value5.
	 *The log and redirect message will be: /putMessage?queueName=value1&message=value2&mId=value3
	 *If succeeds, the return message is "Put message Request Accepted!"
	 *</b>
	 *
	 *@b 4
	 *<b>
	 *  To delete a message, the url should be:
	 *		http:hostName:hostPort/deleteMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=value5.
	 *The log and redirect message will be: /deleteMessage?queueName=value1&mId=value3
	 *If succeeds, the return message is "Delete message Request Accepted!"
	 *</b>
	 *
	 *@b 5
	 *<b>
	 *  To get a message, the url should be:
	 *		http:hostName:hostPort/getMessage?nodeName=value1&nodePort=value2&queueName=value3&mId=value5.
	 *If succeeds, the return message is "Success", otherwise "Fail".
	 *</b>
	 *
	 *@b 6
	 *<b>
	 *To get a recovery mode, the url should be:
	 *		http:hostName:hostPort/recovery?nodeName=value1&nodePort=value2&logsize=val4.
	 *If succeeds, the return message is the log messages.
	 *</b>
	 *
	 *@b 7
	 *<b>
	 *To join it, the url should be:
	 *		http:hostName:hostPort/join?nodeName=value1&nodePort=value2&publicNodeName=val3&publicNodePort=val4.
	 *If succeeds, the return message is "Welcome to join us!".
	 *</b>
	 *
	 *
	 * @param req the object containing all the necessary messages
	 *
	 *@bug
	 * 	if adds "delete url_path;delete url_query;", it will tell me double free the memory.
	 * 	So I do not free it. But who does this job?
	 */
	void onDataNodeRecv (struct evhttp_request *req);

	/**
	 *@todo You can change the return messages!
	 *
	 *@brief
	 * invoke when a request from client received
	 * if the request's path is <b>'/getavailablehost'</b>, it will check the
	 * available data nodes and return one, i.e.
	 *"<b>
	 *nodeName: localhost
	 *nodePort: 8080
	 *</b>"
	 *if no data nodes available, it will return a message:
	 *<b>"No Data Node available..."</b>
	 *
	 *all the other request are  unrecognizable and will return a message:
	 *<b>"Unrecognized request......"</b>
	 *
	 * @param req the object containing all the necessary messages
	 */
	void onClientReqRecv (struct evhttp_request *req);
};



#endif /* SQSMASTER_H_ */
