#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

class sqlite3;

class Database
{
public:
	Database(const char *dbname = "datanode.db");
	virtual ~Database();

	bool createQueue(const char *queueName);
	std::vector<std::string> listQueues();
	bool deleteQueue(const char *queueName);
	int putMessage(const char *queueName, const char *message, int messageID = -1);
	std::string getMessageById(const char *queueName, int messageID, bool &ok);
	std::string getMessage(const char *queueName, int &messageID, bool &ok, int skip = 0);
	bool deleteMessage(const char *queueName, int messageID);

private:
	Database(const Database& other);
	bool hasMessage(const char *queueName, int messageID);
	int messageCount(const char *queueName);
	int minMessageNumber(const char *queueName);

	std::string m_dbname;
	sqlite3 *conn;
};

#endif // DATABASE_H
