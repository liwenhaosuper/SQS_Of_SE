/**
 * Convention.h
 *
 *  Created on: May 15, 2012
 *      Author: liwenhaosuper
 *
 *  Description:
 *      URL conventions for public usage
 */

#ifndef CONVENTION_H_
#define CONVENTION_H_

#include <string>
#include <stdio.h>

using namespace std;
  /**
   * Convention for client handling
   */

string		 		masterIp_ForClient = "127.0.0.1";
int 				masterPort_ForClient = 1200;
string				masterIP_ForDataNode = "127.0.0.1";
int					masterPort_ForDataNode = 1300;

//
/**
 * Url path:
 *    	DataNode to Master
 * localhost:1200/operaion?param=?&param2=?
 * recovery:
 * localhost:1300/recovery?nodeName=%s&nodePort=%s&logSize=%s
 * join:
 * localhost:1300/join?nodeName=%s&nodePort=%s&nodeNameMaster=%s&nodePortMaster=%s
 * createMessage:
 * localhost:1300/putMessage?nodeName=%s&nodePort=%s&nodeNameMaster=%s&nodePortMaster=%s&queueName=%s&message=%s&messageId=%s
 * deleteMessage"
 * localhost:1300/deleteMessage?nodeName=%s&nodePort=%s&nodeNameMaster=%s&nodePortMaster=%s&queueName=%s&messageId=%s
 *
 *
 *		Client to master
 *put message
 *localhost:1300/putMessage?queueName=%s&message=%s
 *delete message
 *localhost:1300/deleteMessage?queueName=%s&messageId=%s
 *get message
 *localhost:1300/deleteMessage?queueName=%s&messageId=%s
 *create queue:
 *localhost:1300/deleteMessage?queueName=%s
 *delete queue:
 *localhost:1300/deleteMessage?queueName=%s
 *
 *
 */




///url path
const string				GET_AVAILABLE_HOST = "/getavailablehost"; /// A client should first call this url to get the available data node to connect
const string				CREATE_QUEUE = "/createQueue";
const string				LIST_QUEUES = "/listQueues";
const string				DEL_QUEUE = "/deleteQueue";
const string				PUT_MSG = "/putMessage";
const string				GET_MSG = "/getMessage";
const string				DELETE_MSG = "/deleteMessage";
const string				EMPTY_PATH = "/";
const string				RECOVERY = "/recovery";
const string				JOIN_TEAM = "/join";
const string				HEARTBEAT = "/heartbeat";

///queue operation

const string				NODE_NAME = "nodeName";
const string				NODE_PORT = "nodePort";

const string				PUBLIC_NODE_NAME = "publicNodeName";
const string				PUBLIC_NODE_PORT = "publicNodePort";

const string				QUEUE_NAME = "queueName";
const string				MSG_ID = "mId";
const string				MSG_LENGTH = "mLength";
const string				MSG_CONTENT = "mData";
const string				MSG = "message";
const string				LOG_SIZE = "logsize";


#endif /* CONVENTION_H_ */
