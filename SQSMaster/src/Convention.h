/*
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
  /*Convention for client handling*/
string		 		masterIp_ForClient = "127.0.0.1";
int 				masterPort_ForClient = 1200;
string				masterIP_ForDataNode = "127.0.0.1";
int					masterPort_ForDataNode = 1300;

//url path
const string				GET_AVAILABLE_HOST = "/getavailablehost"; /* A client should first call this url to get the available data node to connect*/
const string				CREATE_QUEUE = "/createQueue";
const string				LIST_QUEUES = "/listQueues";
const string				DEL_QUEUE = "/deleteQueue";
const string				PUT_MSG = "/putMessage";
const string				GET_MSG = "/getMessage";
const string				DELETE_MSG = "/deleteMessage";
const string				EMPTY_PATH = "/";
const string				RECOVERY = "/recovery";
const string				JOIN_TEAM = "/join";



//queue operation

const string				NODE_NAME = "nodeName";
const string				NODE_PORT = "nodePort";

const string				QUEUE_NAME = "queueName";
const string				MSG_ID = "mId";
const string				MSG_LENGTH = "mLength";
const string				MSG_CONTENT = "mData";



#endif /* CONVENTION_H_ */
