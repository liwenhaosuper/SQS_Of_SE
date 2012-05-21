
#include "SQSClient.h"
#include <iostream>


using namespace std;


int main(){
    cout<<"SQSClient started..."<<endl;
    SQSClient* client = new SQSClient("localhost",1200);
    cout<<"SQSClient stoped..."<<endl;
    return 0;
}
