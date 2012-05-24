#ifndef NODE_H_
#define NODE_H_

#include<err.h>
#include"event.h"
#include"evhttp.h"
#include<string>
#include"Logger.h"
#include"Database.h"

using namespace std;
class Node{
public:
	Node():name(/*gethostname()*/"127.0.0.1"),masterAddress("127.0.0.1"),master_port(1300),logger(new Logger("Node.log")){}
	Node(int masterP,string mastername):name("127.0.0.1"),
			master_port(masterP),masterAddress(mastername),logger(new Logger("Node.log")){}
	~Node(){
		delete logger;
		delete db;
	}
	bool init();
	string getNodeName() const{
		return name;
	}
	int getMasterPort() const{
		return master_port;
	}
	string getMasterAddress() const{
		return masterAddress;
	}
	void dispatch(string request);
	bool Recovery(); //recovery->get new msg from master
	bool Join();     //join the other nodes
	void onMasterRecv(struct evhttp_request *req);//heartbeat call back?
	void onClientRecv(struct evhttp_request *req);//client get msg from node

private:
	string name;    //own node name
	string masterAddress;//master address
	int master_port;//port communicate with master
	int client_port;//port for client
	Logger* logger;  //logger to operate log
	Database* db;
};

#endif
