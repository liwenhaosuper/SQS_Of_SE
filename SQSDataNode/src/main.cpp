#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "DataNode.h"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 3) {
		return -1;
	}

	const char *masterName = "localhost";
	const int masterPort = 1300;
	const char *nodeNamePublic = "localhost";
	const int portPublic = atoi(argv[1]);
	const char *nodeNameMaster = "localhost";
	const int portMaster = atoi(argv[2]);
	const int timeout = 5;
	DataNode *dataNode = new DataNode(masterName, masterPort, nodeNamePublic, portPublic,
					  nodeNameMaster, portMaster, timeout);
	printf("will listen on %d for public and on %d for master.\n", portPublic, portMaster);
	if (!dataNode->start()) {
		cerr << "Fail to start service. Bye" << endl;
	}
	cout << "recovery" << endl;
	dataNode->recovery();
	cout << "join" << endl;
	dataNode->join();
	return 0;
}

// int main()
// {
// 	Database db;
// 	db.createQueue("queueA");
// 	db.createQueue("queueB");
// 	db.createQueue("queueC");
// 	vector<string> list = db.listQueues();
// 	for (int i = 0; i < list.size(); ++i) {
// 		cout << list[i] << endl;
// 	}
// 	db.deleteQueue("queueC");
// 	cout << "size: " << db.listQueues().size() << endl;
// 
// 	int num = -1;
// 	num = db.putMessage("queueA", "hello world");
// 	cout << "num is: " << num << endl;
// 	num = db.putMessage("queueA", "hello world 2", 123);
// 	cout << "num is: " << num << endl;
// 	num = db.putMessage("queueA", "hello world 3");
// 	cout << "num is: " << num << endl;
// 
// 	bool result;
// 	cout << "message: " << db.getMessage("queueA", 1, result) << " result: " << result << endl
// 	     << "message: " << db.getMessage("queueA", 3, result) << " result: " << result << endl;
// 
// 	cout << "result is: " << db.deleteMessage("queueb", 3) << endl
// 	     << "result is: " << db.deleteMessage("queueA", 124) << endl;
// 
// 	return 0;
// }