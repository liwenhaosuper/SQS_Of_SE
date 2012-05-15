//============================================================================
// Name        : SQSMaster.cpp
// Author      : liwenhaosuper
// Version     :
// Copyright   : No rights reserved
// Description : SQS in C++, Ansi-style
//============================================================================

#include <iostream>
#include <err.h>
#include "event.h"
#include "evhttp.h"
#include "SQSMaster.h"

using namespace std;

int main() {
	SQSMaster* master = new SQSMaster(1200,1300,5);
	if(master->init()){
		cout << "SQSMaster is started." << endl;
		master->start();
	}else{
		cout<<"Fail to start service. Bye"<<endl;
	}
	return 0;
}
