#ifndef DATANODE_H
#define DATANODE_H

#include "Logger.h"
#include "Database.h"

#define DATANODE_D 1
typedef void(*dispatchCb)(struct evhttp_request *req, void *arg);

void dispatchMsgCallBack(struct evhttp_request *req, void *arg);

class DataNode
{
	friend void ClientCallBack(struct evhttp_request *req, void *arg);
	friend void MasterCallBack(struct evhttp_request *req, void *arg);
	friend void dispatchMsgCallBack(struct evhttp_request *req, void *arg);
public:
	DataNode(const char *masterName, int masterPort,
		 const char *nodeNamePublic, int portPublic,
		 const char *nodeNameMaster, int portMaster,
		 int timeout
		) : m_masterName(masterName), m_masterPort(masterPort),
		m_nodeNamePublic(nodeNamePublic), m_portPublic(portPublic),
		m_nodeNameMaster(nodeNameMaster), m_portMaster(portMaster),
		m_timeout(timeout), m_logger(new Logger("SQS.log")),
		m_db(new Database()) {}
	virtual ~DataNode() {}

	bool start();
	void recovery();
	void join();

private:
	DataNode(const DataNode &other);

	const char *m_masterName;
	int m_masterPort;
	const char *m_nodeNamePublic;
	int m_portPublic;
	const char *m_nodeNameMaster;
	int m_portMaster;
	int m_timeout;

	Logger *m_logger;
	Database *m_db;

	void onClientRecv(struct evhttp_request *req);
	void onMasterRecv(struct evhttp_request *req);
	void dispatchMessage(const char *remoteNode, int remotePort, const char *request, dispatchCb callback = dispatchMsgCallBack, void *param = NULL);
	void dispatchCheckLock(const char *remoteNode, int remotePort, const char *request, bool *unlock);

};

#endif // DATANODE_H
