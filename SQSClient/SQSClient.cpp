#include <vector>

#include <iostream>
#include <string.h>

#include "event.h"
#include "evhttp.h"

#include <cstdlib>

#include "SQSClient.h"

#include <map>

using namespace std;

struct Param{
    char* rsp;
    struct event_base* base;
};

void response_callback(struct evhttp_request *req, void *rsp){
    Param* param = (Param*)rsp;
    if(param==NULL){
        return;
    }
    if(req==NULL||req->response_code==0){
        param->rsp = NULL;
    }else{
        struct evbuffer *buf = evhttp_request_get_input_buffer(req);
        size_t sz = evbuffer_get_length(buf);
        if(sz<=0){
            param->rsp = "";
        }else{
            param->rsp = new char[sz+1];
            evbuffer_remove(buf,param->rsp,sz);
        }
    }
    event_base_loopbreak(param->base);
}


char* SQSClient::doRequest(std::string dataNode,int port,std::string path){
    struct event_base *base = event_base_new();//TODO: Should I delete you?
    struct evhttp_connection *cn = evhttp_connection_base_new(//TODO: Should I delete you?
            base, NULL,
            dataNode.c_str(),
            port);
    Param* param = (struct Param*)malloc(sizeof(struct Param));
    param->base = base;param->rsp = NULL;
    struct evhttp_request *req = evhttp_request_new(response_callback,param);
    evhttp_make_request(cn,req,EVHTTP_REQ_GET,path.c_str());
    event_base_dispatch(base);
    if(param->rsp==NULL){
        free(param);
        return NULL;
    }
    char* res = param->rsp;
    param->rsp = NULL;
    free(param);
    return res;
}



bool SQSClient::getRemoteHost(){
    char* rsp;
    rsp = doRequest(this->masterName,this->masterPort,"/getavailablehost");
    if(rsp==NULL){
        cout<<"WOW!rsp is null..."<<endl;
        return false;
    }else{
        cout<<rsp<<endl;
        if(strcmp(rsp,"No Data Node available")==0){
            return false;
        }else{
            //TODO: Parse the string
            string container = rsp;
            map<string,string> data;
            int begin = 0,end=0;
            while((end = container.find_first_of(":",begin))>=0){
                string key = container.substr(begin,end-begin);
                begin = end;
                end = container.find_first_of(":",begin);
                string value;
                if(end<=0){
                    value = container.substr(end);
                }else{
                    value = container.substr(begin,end-begin);
                    begin = end;
                }
                data.insert(make_pair(key,value));
            }
            map<string,string>::iterator iter = data.find("nodeName");
            if(iter!=data.end()){
                this->dataNodeName = iter->first;
                this->dataNodePort = atoi(iter->second.c_str());
                this->isDataNodeReady = true;
                return true;
            }
            return false;
        }

    }

}

/*!
 *@brief create a queue
 *@param QueueName the queue name to be created
 *@return true if successfully created, otherwise false
 */
bool SQSClient::CreateQueue(std::string QueueName){
    int cnt = 0;
    while(!isDataNodeReady){
        getRemoteHost();
        if(isDataNodeReady){
            break;
        }
        cnt++;
        if(cnt>3){
            return false;
        }
    }
    char* rsp;
    rsp = doRequest(this->dataNodeName,this->dataNodePort,"/createQueue?&queueName="+QueueName);
    cnt = 0;
    while(rsp==NULL||strcmp(rsp,"")==0){
        cnt++;
        if(cnt>3){
            this->isDataNodeReady = false;
            getRemoteHost();
            if(this->isDataNodeReady){
               cnt = 0;
            }else{
                return false;
            }
        }
        rsp = doRequest(this->dataNodeName,this->dataNodePort,"/createQueue?&queueName="+QueueName);
    }
    //TODO: do more parsing
    return true;
}

/*!
 *@brief list all the queue names
 *@return the vector containing all the queue names or
 *      null when fails to connect to data node
 */
vector<string> *SQSClient::ListQueues(){
    int cnt = 0;
    while(!isDataNodeReady){
        getRemoteHost();
        if(isDataNodeReady){
            break;
        }
        cnt++;
        if(cnt>3){
            return NULL;
        }
    }
    char* rsp;
    rsp = doRequest(this->dataNodeName,this->dataNodePort,"/listQueues");
    cnt = 0;
    while(rsp==NULL||strcmp(rsp,"")==0){
        cnt++;
        if(cnt>3){
            this->isDataNodeReady = false;
            getRemoteHost();
            if(this->isDataNodeReady){
               cnt = 0;
            }else{
                return NULL;
            }
        }
        rsp = doRequest(this->dataNodeName,this->dataNodePort,"/listQueues");
    }
    vector<string>* res = new vector<string>;
    //TODO: parse the results
    //TO BE CONTINUED...
    return res;
}

/*!
 *@brief delete the queue via its queue name
 *@return true if successfully deleted otherwise false
 */
bool SQSClient::DeleteQueue(std::string QueueName){
    int cnt = 0;
    while(!isDataNodeReady){
        getRemoteHost();
        if(isDataNodeReady){
            break;
        }
        cnt++;
        if(cnt>3){
            return false;
        }
    }
    char* rsp;
    rsp = doRequest(this->dataNodeName,this->dataNodePort,"/deleteQueue?&queueName="+QueueName);
    cnt = 0;
    while(rsp==NULL||strcmp(rsp,"")==0){
        cnt++;
        if(cnt>3){
            this->isDataNodeReady = false;
            getRemoteHost();
            if(this->isDataNodeReady){
               cnt = 0;
            }else{
                return false;
            }
        }
        rsp = doRequest(this->dataNodeName,this->dataNodePort,"/deleteQueue?&queueName="+QueueName);
    }
    //TODO: some more parsing
    return true;
}

/*!
 *@brief add a message to a queue
 *@param QueueName the queue name  that the message to be inserted
 *@param Message the message to be inserted
 *@return true of successfully inserted otherwise false
 */
bool SQSClient::SendMessage(std::string QueueName, std::string Message){
    int cnt = 0;
    while(!isDataNodeReady){
        getRemoteHost();
        if(isDataNodeReady){
            break;
        }
        cnt++;
        if(cnt>3){
            return false;
        }
    }
    char* rsp;
    rsp = doRequest(this->dataNodeName,this->dataNodePort,"/putMessage?&queueName="+QueueName+"&message="+Message);
    cnt = 0;
    while(rsp==NULL||strcmp(rsp,"")==0){
        cnt++;
        if(cnt>3){
            this->isDataNodeReady = false;
            getRemoteHost();
            if(this->isDataNodeReady){
               cnt = 0;
            }else{
                return false;
            }
        }
        rsp = doRequest(this->dataNodeName,this->dataNodePort,"/putMessage?&queueName="+QueueName+"&message="+Message);
    }
    //TODO: some more parsing
    return true;
}

/*!
 *@brief get a message from a queue
 *@param QueueName the queue name to get the message
 *@param MessageID the message id that belongs to the return message,-1 if no message got
 *@return the message if success, otherwise return ""
 */
std::string SQSClient::ReceiveMessage(std::string QueueName, int &MessageID){
    int cnt = 0;
    while(!isDataNodeReady){
        getRemoteHost();
        if(isDataNodeReady){
            break;
        }
        cnt++;
        if(cnt>3){
            return "";
        }
    }
    char* rsp;
    rsp = doRequest(this->dataNodeName,this->dataNodePort,"/getMessage?&queueName="+QueueName);
    cnt = 0;
    while(rsp==NULL||strcmp(rsp,"")==0){
        cnt++;
        if(cnt>3){
            this->isDataNodeReady = false;
            getRemoteHost();
            if(this->isDataNodeReady){
               cnt = 0;
            }else{
                return "";
            }
        }
        rsp = doRequest(this->dataNodeName,this->dataNodePort,"/getMessage?&queueName="+QueueName);
    }
    string container = rsp;
    map<string,string> data;
    int begin = 0,end=0;
    while((end = container.find_first_of(":",begin))>=0){
        string key = container.substr(begin,end-begin);
        begin = end;
        end = container.find_first_of(":",begin);
        string value;
        if(end<=0){
            value = container.substr(end);
        }else{
            value = container.substr(begin,end-begin);
            begin = end;
        }
        data.insert(make_pair(key,value));
    }
    map<string,string>::iterator iter = data.find("mId");
    if(iter!=data.end()){
        MessageID = atoi(iter->second.c_str());
    }
    iter = data.find("message");
    if(iter!=data.end()){
       return iter->second;
    }
}



/*!
 *@brief delete a message from a queue with a message id
 *@param QueueName the queue name that the message belongs to
 *@param MessageID the message id to be deleted
 *@return true if success , otherwise false
 */
bool SQSClient::DeleteMessage(std::string QueueName,int MessageID){
    int cnt = 0;
    while(!isDataNodeReady){
        getRemoteHost();
        if(isDataNodeReady){
            break;
        }
        cnt++;
        if(cnt>3){
            return false;
        }
    }
    char* rsp;
    rsp = doRequest(this->dataNodeName,this->dataNodePort,"/deleteQueue?&queueName="+QueueName);
    cnt = 0;
    while(rsp==NULL||strcmp(rsp,"")==0){
        cnt++;
        if(cnt>3){
            this->isDataNodeReady = false;
            getRemoteHost();
            if(this->isDataNodeReady){
               cnt = 0;
            }else{
                return false;
            }
        }
        rsp = doRequest(this->dataNodeName,this->dataNodePort,"/deleteQueue?&queueName="+QueueName);
    }
    //TODO: some more parsing
    return true;
}
