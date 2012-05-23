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

	bool createQueue(const std::string &queueName);
	std::vector<std::string> listQueue();
	bool deleteQueue(const std::string &queueName);
	int putMessage(const std::string& queueName, const std::string& message, int messageID = -1);
	std::string getMessage(const std::string &queueName, int messageID, bool &ok);
	bool deleteMessage(const std::string &queueName, int messageID);

private:
	Database(const Database& other);
	bool hasMessage(const std::string &queueName, int messageID);

	std::string m_dbname;
	sqlite3 *conn;
};

#endif // DATABASE_H
