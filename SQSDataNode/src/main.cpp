#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <string>
#include <getopt.h>
#include "DataNode.h"

using namespace std;

static struct option long_options[] = {
	{"masterName",     required_argument, 0, 0},
	{"masterPort",     required_argument, 0, 0},
	{"nodeName",       required_argument, 0, 0},
	{"nodePort",       required_argument, 0, 0},
	{"nodeNameMaster", required_argument, 0, 0},
	{"nodePortMaster", required_argument, 0, 0},
	{"timeout",        required_argument, 0, 0},
	{0,                0,                 0, 0}
};

void showUsage()
{
	const char *b = "-------------------------------------------------------------------------------\n"
			"--masterName     <name>  master node's name\n"
			"--masterPort     <port>  master node's port\n"
			"--nodeName       <name>  the node's name for client\n"
			"--nodePort       <port>  the node's port for client\n"
			"--nodeNameMaster <name>  the node's name for master\n"
			"--nodePortMaster <port>  the node's port for master\n"
			"--timeout        <num>   keep-alive timeout for an http request (default: 60)\n"
			"-h                       print this help and exit\n"
			"-------------------------------------------------------------------------------\n";
	printf("%s", b);
}

int main(int argc, char *argv[])
{
	char name[65];
	gethostname(name, sizeof(name));
	const char *masterName = name;
	int masterPort = 1300;
	const char *nodeNamePublic = name;
	int portPublic = 1500;
	const char *nodeNameMaster = name;
	int portMaster = 1600;
	int timeout = 60;

	int c;

	while (1) {
		int option_index = 0;
		c = getopt_long(argc, argv, "h", long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 0:
			switch (option_index) {
			case 0:
				masterName = optarg;
				break;
			case 1:
				masterPort = atoi(optarg);
				break;
			case 2:
				nodeNamePublic = optarg;
				break;
			case 3:
				portPublic = atoi(optarg);
				break;
			case 4:
				nodeNameMaster = optarg;
				break;
			case 5:
				portMaster = atoi(optarg);
				break;
			case 6:
				timeout = atoi(optarg);
				break;
			default:
				break;
			}
			break;
		case 'h':
		default:
			showUsage();
			exit(1);
			break;
		}
	}

	DataNode *dataNode = new DataNode(masterName, masterPort, nodeNamePublic, portPublic,
					  nodeNameMaster, portMaster, timeout);
	printf("will listen on %d for public and on %d for master.\n", portPublic, portMaster);
	if (!dataNode->start()) {
		fprintf(stderr, "Fail to start service. Bye\n");
		return 1;
	}
	return 0;
}

// #include <vector>
// #include <list>
// #include <iostream>
// using namespace std;
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
// 	cout << "message: " << db.getMessage("queueA", result) << " result: " << result << endl
// 	     << "message: " << db.getMessage("queueb", result) << " result: " << result << endl;
// 
// 	cout << "result is: " << db.deleteMessage("queueb", 3) << endl
// 	     << "result is: " << db.deleteMessage("queueA", 124) << endl
// 	     << "result is: " << db.deleteMessage("queueA", 1) << endl;
// 
// 	cout << "message: " << db.getMessage("queueA", result) << " result: " << result << endl;
// 
// 	return 0;
// }
