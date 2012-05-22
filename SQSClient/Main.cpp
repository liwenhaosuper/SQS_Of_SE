
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
    client->getRemoteHost();
    time(&t2);
    cout<<"time cost:(s)"<<t2-t1<<endl;
    cout<<"SQSClient stoped..."<<endl;
    return 0;
}

