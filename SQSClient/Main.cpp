
#include "SQSClient.h"
#include <iostream>

#include <sys/time.h>

#include <string.h>
#include <cstdlib>

using namespace std;


int main(){
    cout<<"SQSClient started..."<<endl;
    time_t t1,t2;
    time(&t1);
    SQSClient* client = new SQSClient("localhost",1200);
    //client->getRemoteHost();
//    client->CreateQueue("helloWorld");
//    client->SendMessage("helloWorld","msg1");
//    client->SendMessage("helloWorld","msg1");
//    client->SendMessage("helloWorld","msg1");
//    client->SendMessage("helloWorld","msg1");//client->ListQueues();
    //client->DeleteQueue("helloWorld");
    //client->ListQueues();
    int id;
    cout<<"Recv Msg:"<<client->ReceiveMessage("helloWorld",id)<<endl;
    cout<<"Msg id:"<<id<<endl;
    //client->DeleteMessage("helloWorld",id);
    //cout<<"Recv Msg:"<<client->ReceiveMessage("helloWorld",id)<<endl;
    time(&t2);
    cout<<"time cost:(s)    "<<t2-t1<<endl;
    cout<<"SQSClient stoped..."<<endl;
    return 0;
}

