#include "DataNode.h"

#include <event.h>
#include <evhttp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <pthread.h>

#include "Convention.h"

#define safe_delete(p) \
{\
 if ((p) != NULL)\
 {\
  delete (p);\
  (p) = NULL;\
 }\
}

#define safe_free(p) \
{\
 if ((p) != NULL)\
 {\
  free((p));\
  (p) = NULL;\
 }\
}

void ClientCallBack(struct evhttp_request *req, void *arg)
{
	DataNode *obj = (DataNode *)arg;
	obj->onClientRecv(req);
}

void MasterCallBack(struct evhttp_request *req, void *arg)
{
	DataNode *obj = (DataNode *)arg;
	obj->onMasterRecv(req);
}

void dispatchMsgCallBack(struct evhttp_request *req, void *arg)
{
	event_base_loopbreak((struct event_base *)arg);
}

void* recover_and_join(void *ptr)
{
	DataNode *node = (DataNode *)ptr;
	printf("start recovery\n");
	node->recovery();
	printf("start join\n");
	node->join();
	return (void *)0;
}

void DataNode::dispatchMessage(const char *remoteNode, int remotePort, const char *request)
{
	struct event_base *base = event_base_new();
	struct evhttp_connection *cn = evhttp_connection_base_new(
		base, NULL, remoteNode, remotePort
	);
	struct evhttp_request *req = evhttp_request_new(dispatchMsgCallBack, base);
	if (evhttp_make_request(cn, req, EVHTTP_REQ_GET, request) == -1) {
		fprintf(stderr, "Make request fail...\n");
	}
	event_base_dispatch(base);
}

bool DataNode::start()
{
	event_init();
	struct evhttp *httpd_client, *httpd_master;
	httpd_client = evhttp_start("0.0.0.0", m_portPublic);
	httpd_master = evhttp_start("0.0.0.0", m_portMaster);
	if (!httpd_client) {
		fprintf(stderr, "ERROR:Unable to listen on %s:%d\n","0.0.0.0", m_portPublic);
		return false;
	}
	if (!httpd_master) {
		fprintf(stderr, "ERROR:Unable to listen on %s:%d\n","0.0.0.0", m_portMaster);
		return false;
	}
	evhttp_set_timeout(httpd_client, m_timeout);
	evhttp_set_timeout(httpd_master, m_timeout);
	evhttp_set_gencb(httpd_client, ClientCallBack, this);
	evhttp_set_gencb(httpd_master, MasterCallBack, this);

	// create a new thread to do recovery work
	pthread_t pid;
	pthread_create(&pid, NULL, recover_and_join, this);

	event_dispatch();
	return true;
}

void DataNode::recovery()
{
	char m_portMasterStr[16];
	sprintf(m_portMasterStr, "%d", m_portMaster);
	char logSize[16];
	sprintf(logSize, "%d", m_logger->count());
	string command = RECOVERY;
	command += "?" + NODE_NAME + "=" + m_nodeNameMaster
		    + "&" + NODE_PORT + "=" + m_portMasterStr
		    + "&" + LOG_SIZE + "=" + logSize;
	dispatchMessage(m_masterName, m_masterPort, command.c_str());
}

void DataNode::join()
{
	char m_portMasterStr[16];
	sprintf(m_portMasterStr, "%d", m_portMaster);
	char m_portPublicStr[16];
	sprintf(m_portPublicStr, "%d", m_portPublic);
	string command = JOIN_TEAM;
	command += "?" + NODE_NAME + "=" + m_nodeNameMaster
		    + "&" + NODE_PORT + "=" + m_portMasterStr
		    + "&" + PUBLIC_NODE_NAME + "=" + m_nodeNamePublic
		    + "&" + PUBLIC_NODE_PORT + "=" + m_portPublicStr;
	dispatchMessage(m_masterName, m_masterPort, command.c_str());
}

void DataNode::onClientRecv(evhttp_request *req)
{
	printf("Receive msg from client.path:%s\n", req->uri);
	struct evbuffer *buf;
	buf = evbuffer_new();
	/* parse path and URL paramter */
	struct evkeyvalq *url_parameters = (struct evkeyvalq *)malloc(sizeof(evkeyvalq));
	const char *url_path = evhttp_uri_get_path(req->uri_elems);
	const char *url_query = evhttp_uri_get_query(req->uri_elems);
	evhttp_parse_query_str(url_query, url_parameters);

	if (!url_parameters) {
		evbuffer_free(buf);
		safe_free(url_parameters);
		return;
	}

	if (strcmp(url_path, CREATE_QUEUE.c_str()) == 0) { /*create queue*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		if (!queueName) {
			evbuffer_add_printf(buf, "%s","Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		// store the change
		if (!m_db->createQueue(queueName)) {
			evbuffer_add_printf(buf, "%s", "Create queue fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = CREATE_QUEUE;
		command += "?" + QUEUE_NAME + "=" + queueName;
		m_logger->addLog(command);

		// send message to master
		char m_portMasterStr[16];
		sprintf(m_portMasterStr, "%d", m_portMaster);
		string toMaster = CREATE_QUEUE;
		toMaster += "?" + NODE_NAME + "=" + m_nodeNameMaster
			   + "&" + NODE_PORT + "=" + m_portMasterStr
			   + "&" + QUEUE_NAME + "=" + queueName;
		dispatchMessage(m_masterName, m_masterPort, toMaster.c_str());

		evbuffer_add_printf(buf, "%s", "Create queue successfully");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path, LIST_QUEUES.c_str()) == 0) { /*list queues*/
		vector<string> queueList = m_db->listQueues();

		string listStr;
		size_t listCnt = queueList.size();
		for (size_t i = 0; i < listCnt; ++i)
			listStr += queueList[i] + "\r\n";

		evbuffer_add_printf(buf, "%s", listStr.c_str());
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path, DEL_QUEUE.c_str()) == 0) { /*delete queue*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		if (!queueName) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		// store the change
		if (!m_db->deleteQueue(queueName)) {
			evbuffer_add_printf(buf, "%s", "Delete queue fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = DEL_QUEUE;
		command += "?" + QUEUE_NAME + "=" + queueName;
		m_logger->addLog(command);

		// send message to master
		char m_portMasterStr[16];
		sprintf(m_portMasterStr, "%d", m_portMaster);
		string toMaster = DEL_QUEUE;
		toMaster += "?" + NODE_NAME + "=" + m_nodeNameMaster
			   + "&" + NODE_PORT + "=" + m_portMasterStr
			   + "&" + QUEUE_NAME + "=" + queueName;
		dispatchMessage(m_masterName, m_masterPort, toMaster.c_str());

		evbuffer_add_printf(buf, "%s", "Delete queue successfully");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path, PUT_MSG.c_str()) == 0) { /*put message*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		const char *msg = evhttp_find_header(url_parameters, MSG.c_str());
		if (!queueName || !msg) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		// store the change
		int msgId = m_db->putMessage(queueName, msg);
		if (msgId == -1) {
			evbuffer_add_printf(buf, "%s", "Put message fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(msg);
			safe_delete(url_parameters);
			return;
		}

		// add log
		char msgIdStr[16];
		sprintf(msgIdStr, "%d", msgId);
		string command = PUT_MSG;
		command += "?" + QUEUE_NAME + "=" + queueName + "&?" + MSG + "=" + msg + "&" + MSG_ID + "=" + msgIdStr;
		m_logger->addLog(command);

		// send message to master
		char m_portMasterStr[16];
		sprintf(m_portMasterStr, "%d", m_portMaster);
		string toMaster = PUT_MSG;
		toMaster += "?" + NODE_NAME + "=" + m_nodeNameMaster
			   + "&" + NODE_PORT + "=" + m_portMasterStr
			   + "&" + QUEUE_NAME + "=" + queueName
			   + "&" + MSG + "=" + msg
			   + "&" + MSG_ID + "=" + msgIdStr;
		dispatchMessage(m_masterName, m_masterPort, toMaster.c_str());

		evbuffer_add_printf(buf, "%d", msgId);
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(msg);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path,DELETE_MSG.c_str()) == 0) { /*delete message*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		const char *msgIdStr = evhttp_find_header(url_parameters, MSG_ID.c_str());
		if (!queueName || !msgIdStr) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		int msgId = atoi(msgIdStr);

		// store the change
		if (!m_db->deleteMessage(queueName, msgId)) {
			evbuffer_add_printf(buf, "%s", "Delete message fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(msgIdStr);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = DELETE_MSG;
		command += "?" + QUEUE_NAME + "=" + queueName + "&" + MSG_ID + "=" + msgIdStr;
		m_logger->addLog(command);

		// send message to master
		char m_portMasterStr[16];
		sprintf(m_portMasterStr, "%d", m_portMaster);
		string toMaster = DELETE_MSG;
		toMaster += "?" + NODE_NAME + "=" + m_nodeNameMaster
			   + "&" + NODE_PORT + "=" + m_portMasterStr
			   + "&" + QUEUE_NAME + "=" + queueName
			   + "&" + MSG_ID + "=" + msgIdStr;
		dispatchMessage(m_masterName, m_masterPort, toMaster.c_str());

		evbuffer_add_printf(buf, "%s", "Delete message successfully");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(msgIdStr);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path, GET_MSG.c_str()) == 0) { /*get message*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		const char *msgIdStr = evhttp_find_header(url_parameters, MSG_ID.c_str());
		if (!queueName || !msgIdStr) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		int msgId = atoi(msgIdStr);

		// store the change
		bool ok = false;
		string msg = m_db->getMessage(queueName, msgId, ok);
		if (!ok) {
			evbuffer_add_printf(buf, "%s", "Get message fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(msgIdStr);
			safe_delete(url_parameters);
			return;
		}

		evbuffer_add_printf(buf, "%s", msg.c_str());
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(msgIdStr);
		safe_delete(url_parameters);
		return;
	} else {
		// hacker?
		evbuffer_add_printf(buf, "%s", "Unrecognized request");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(url_parameters);
		return;
	}
}

void DataNode::onMasterRecv(evhttp_request *req)
{
	printf("Receive msg from master:%s\n", req->uri);
	struct evbuffer *buf;
	buf = evbuffer_new();
	/* parse path and URL paramter */
	struct evkeyvalq *url_parameters = (struct evkeyvalq *)malloc(sizeof(evkeyvalq));
	const char *url_path = evhttp_uri_get_path(req->uri_elems);
	const char *url_query = evhttp_uri_get_query(req->uri_elems);
	evhttp_parse_query_str(url_query, url_parameters);

	if (!url_parameters) {
		evbuffer_free(buf);
		safe_free(url_parameters);
		return;
	}

	if (strcmp(url_path, CREATE_QUEUE.c_str()) == 0) { /*create queue*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		if (!queueName) {
			evbuffer_add_printf(buf, "%s","Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		// store the change
		if (!m_db->createQueue(queueName)) {
			evbuffer_add_printf(buf, "%s", "Create queue fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = CREATE_QUEUE;
		command += "?" + QUEUE_NAME + "=" + queueName;
		m_logger->addLog(command);

		evbuffer_add_printf(buf, "%s", "Create queue successfully");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path, DEL_QUEUE.c_str()) == 0) { /*delete queue*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		if (!queueName) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		// store the change
		if (!m_db->deleteQueue(queueName)) {
			evbuffer_add_printf(buf, "%s", "Delete queue fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = DEL_QUEUE;
		command += "?" + QUEUE_NAME + "=" + queueName;
		m_logger->addLog(command);

		evbuffer_add_printf(buf, "%s", "Delete queue successfully");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path, PUT_MSG.c_str()) == 0) { /*put message*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		const char *msg = evhttp_find_header(url_parameters, MSG.c_str());
		const char *msgIdStr = evhttp_find_header(url_parameters, MSG_ID.c_str());
		if (!queueName || !msg) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		int msgId = atoi(msgIdStr);

		// store the change
		int msgIdResult = m_db->putMessage(queueName, msg, msgId);
		if (msgId != msgIdResult) {
			evbuffer_add_printf(buf, "%s", "Put message fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(msg);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = PUT_MSG;
		command += "?" + QUEUE_NAME + "=" + queueName + "&?" + MSG + "=" + msg + "&" + MSG_ID + "=" + msgIdStr;
		m_logger->addLog(command);

		evbuffer_add_printf(buf, "%d", msgId);
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(msg);
		safe_delete(url_parameters);
		return;
	} else if (strcmp(url_path,DELETE_MSG.c_str()) == 0) { /*delete message*/
		const char *queueName = evhttp_find_header(url_parameters, QUEUE_NAME.c_str());
		const char *msgIdStr = evhttp_find_header(url_parameters, MSG_ID.c_str());
		if (!queueName || !msgIdStr) {
			evbuffer_add_printf(buf, "%s", "Unrecognized request");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(url_parameters);
			return;
		}

		int msgId = atoi(msgIdStr);

		// store the change
		if (!m_db->deleteMessage(queueName, msgId)) {
			evbuffer_add_printf(buf, "%s", "Delete message fail");
			evhttp_send_reply(req, HTTP_OK, "OK", buf);
			evbuffer_free(buf);
			safe_delete(queueName);
			safe_delete(msgIdStr);
			safe_delete(url_parameters);
			return;
		}

		// add log
		string command = DELETE_MSG;
		command += "?" + QUEUE_NAME + "=" + queueName + "&" + MSG_ID + "=" + msgIdStr;
		m_logger->addLog(command);

		evbuffer_add_printf(buf, "%s", "Delete message successfully");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(queueName);
		safe_delete(msgIdStr);
		safe_delete(url_parameters);
		return;
	} else {
		// hacker?
		evbuffer_add_printf(buf, "%s", "Unrecognized request");
		evhttp_send_reply(req, HTTP_OK, "OK", buf);
		evbuffer_free(buf);
		safe_delete(url_parameters);
		return;
	}
}
