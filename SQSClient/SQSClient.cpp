#include <vector>

#include <iostream>
#include <string.h>

#include "event.h"
#include "evhttp.h"

#include <cstdlib>

#include "SQSClient.h"

using namespace std;

void response_callback(struct evhttp_request *req, void *rsp){
    memcpy(rsp,req,sizeof(struct evhttp_request));
}

struct evhttp_request* SQSClient::doRequest(std::string dataNode,int port,std::string path){
    struct event_base *base = event_base_new();
    struct evhttp_connection *cn = evhttp_connection_base_new(
            base, NULL,
            dataNode.c_str(),
            port);
    struct evhttp_request *rsp = NULL;
    /* Allocate request structure */
    if ((rsp = (struct evhttp_request *)malloc(sizeof(struct evhttp_request))) == NULL) {
        cout<<"Error allocating rsp structure"<<endl;
        return NULL;
    }
    struct evhttp_request *req = evhttp_request_new(response_callback,rsp);
    evhttp_make_request(cn,req,EVHTTP_REQ_GET,path.c_str());
    event_base_dispatch(base);
    return rsp;
}


bool SQSClient::getRemoteHost(){
    return false;
}

/*!
 *@brief create a queue
 *@param QueueName the queue name to be created
 *@return true if successfully created, otherwise false
 */
bool SQSClient::CreateQueue(std::string QueueName){
    return false;
}

/*!
 *@brief list all the queue names
 *@return the vector containing all the queue names or
 *      null when fails to connect to data node
 */
std::vector<std::string> SQSClient::ListQueues(){
    std::vector<std::string> res;
    return res;
}

/*!
 *@brief delete the queue via its queue name
 *@return true if successfully deleted otherwise false
 */
bool SQSClient::DeleteQueue(std::string QueueName){
    return false;
}

/*!
 *@brief add a message to a queue
 *@param QueueName the queue name  that the message to be inserted
 *@param Message the message to be inserted
 *@return true of successfully inserted otherwise false
 */
bool SQSClient::SendMessage(std::string QueueName, std::string Message){
    return false;
}

/*!
 *@brief get a message from a queue
 *@param QueueName the queue name to get the message
 *@param MessageID the message id that belongs to the return message,-1 if no message got
 *@return the message if success, otherwise return ""
 */
std::string SQSClient::ReceiveMessage(std::string QueueName, int &MessageID){
    return "";
}

/*!
 *@brief delete a message from a queue with a message id
 *@param QueueName the queue name that the message belongs to
 *@param MessageID the message id to be deleted
 *@return true if success , otherwise false
 */
bool SQSClient::DeleteMessage(std::string QueueName,int MessageID){
    return false;
}
