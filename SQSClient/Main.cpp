
#include "SQSClient.h"
#include <iostream>

#include <sys/time.h>

#include <string.h>
#include <cstdlib>

using namespace std;

void createQueueAndMsg(SQSClient* client){
    cout<<"Creating queue and message..."<<endl;
    cout<<"Creating queue helloWorld0.."<<endl;
    client->CreateQueue("helloWorld0");
    client->SendMessage("helloWorld0","msg1");
    client->SendMessage("helloWorld0","msg2");
    client->SendMessage("helloWorld0","msg3");
    client->SendMessage("helloWorld0","msg4");
    client->SendMessage("helloWorld0","msg5");
    client->SendMessage("helloWorld0","msg6");
    client->SendMessage("helloWorld0","msg7");
    client->SendMessage("helloWorld0","msg8");
    cout<<"Creating queue helloWorld1.."<<endl;
    client->CreateQueue("helloWorld1");
    client->SendMessage("helloWorld1","msg11");
    client->SendMessage("helloWorld1","msg12");
    client->SendMessage("helloWorld1","msg13");
    client->SendMessage("helloWorld1","msg14");
    cout<<"Creating queue helloWorld2.."<<endl;
    client->CreateQueue("helloWorld2");
    client->SendMessage("helloWorld2","msg21");
    client->SendMessage("helloWorld2","msg22");
    client->SendMessage("helloWorld2","msg23");
    client->SendMessage("helloWorld2","msg24");
}
void getMessage(SQSClient* client){
    int mid;
    string rsp;
    cout<<"Get message..."<<endl;
    for(int i=0;i<20;i++){
        rsp = client->ReceiveMessage("helloWorld0",mid);
        cout<<"Get Message:"<<rsp<<",mId:"<<mid<<endl;
        if(rsp.compare("")==0){
            break;
        }
    }
    for(int i=0;i<20;i++){
        rsp = client->ReceiveMessage("helloWorld1",mid);
        cout<<"Get Message:"<<rsp<<",mId:"<<mid<<endl;
        if(rsp.compare("")==0){
            break;
        }
    }
}
void getAndDelete(SQSClient* client){
    int mid;
    string rsp;
    cout<<"Get and Delete Message..."<<endl;
    for(int i=0;i<20;i++){
        rsp = client->ReceiveMessage("helloWorld1",mid);
        cout<<"Get Message And Delete:"<<rsp<<",mId:"<<mid<<endl;
        client->DeleteMessage("helloWorld1",mid);
        if(rsp.compare("")==0||mid==-1){
            break;
        }
        mid = -1;
    }
    for(int i=0;i<20;i++){
        rsp = client->ReceiveMessage("helloWorld2",mid);
        cout<<"Get Message And Delete:"<<rsp<<",mId:"<<mid<<endl;
        client->DeleteMessage("helloWorld1",mid);
        if(rsp.compare("")==0){
            break;
        }
        mid = -1;
    }
}

void listQueues(SQSClient* client){
    vector<string>* vec = client->ListQueues();
    cout<<"Queue list:";
    for(int i=0;i<vec->size();i++){
        if(i==vec->size()-1) cout<<vec->at(i)<<endl;
        else cout<<vec->at(i)<<",";
    }

}
void deleteQueues(SQSClient* client){
    cout<<"Deleting helloWorld2..."<<endl;
    client->DeleteQueue("helloWorld2");
    listQueues(client);
}


int main(){
    cout<<"SQSClient started..."<<endl;
    time_t t1,t2;
    time(&t1);
    SQSClient* client = new SQSClient("localhost",1200);
    createQueueAndMsg(client);
    getMessage(client);
    getAndDelete(client);
    listQueues(client);
    deleteQueues(client);
    time(&t2);
    cout<<"time cost:(s)    "<<t2-t1<<endl;
    return 0;
}

