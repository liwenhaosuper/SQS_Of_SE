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

string				GET_AVAILABLE_HOST = "/getavailablehost";

string				CREATE_QUEUE_REQ = "/createQueue?queueName=%s";
string				LIST_QUEUES = "/listQueues";
string				SEND_MESSAGE = "/?queueName=%s&opt=put&dataLength=%s&data=%s";//data content will be followed
string				GET_MESSAGE = "/?queueName=%s&opt=get";
string				DELETE_MESSAGE = "/?queueName=%s&opt=del&mId=%s";
string				DeleteQUEUE = "/deleteQueue?queueName=%s";

   /*Convention for data node handling*/
string				masterIP_ForDataNode = "127.0.0.1";
int					masterPort_ForDataNode = 1300;

string				RECOVERY_MESSAGE = "/recovery?logid=%s";  /*a data node should first get into the cecovery mode before starting to accept request*/
string				NORMAL_JOIN = "/join?ip=%s&port=%s";  /* a data node should tell the master to start process data after finishing recovery*/




#endif /* CONVENTION_H_ */
