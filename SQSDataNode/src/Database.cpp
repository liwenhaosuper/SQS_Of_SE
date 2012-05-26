#include "Database.h"

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <sqlite3.h>

using namespace std;

static int listQueueCallback(void *data, int n_columns, char **col_values, char **col_names);
static int getMessageCallback(void *data, int n_columns, char **col_values, char **col_names);

Database::Database(const char *dbname) : m_dbname(dbname)
{
	if (sqlite3_open(m_dbname.c_str(), &conn) != SQLITE_OK) {
		cerr << "can't open the database.";
		exit(-1);
	}
}

Database::~Database()
{
	if (sqlite3_close(conn) != SQLITE_OK) {
		cerr << "can't close the database: " << sqlite3_errmsg(conn) << endl;
		exit(-1);
	}
}

bool Database::createQueue(const char *queueName)
{
	char *err_msg = NULL;
	char sql[128];
	sprintf(sql, "CREATE TABLE [%s] ([id] INTEGER PRIMARY KEY AUTOINCREMENT, [message] TEXT);", queueName);
	if (sqlite3_exec(conn, sql, 0, 0, &err_msg) != SQLITE_OK)
		return false;
	else
		return true;
}

vector<string> Database::listQueues()
{
	char *err_msg = NULL;
	vector<string> result;

	const char *sql = "SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
	if (sqlite3_exec(conn, sql, &listQueueCallback, &result, &err_msg) != SQLITE_OK) {
		vector<string> error;
		return error;
	} else {
		return result;
	}
}

bool Database::deleteQueue(const char *queueName)
{
	char *err_msg = NULL;
	char sql[128];
	sprintf(sql, "DROP TABLE [%s];", queueName);
	if (sqlite3_exec(conn, sql, 0, 0, &err_msg) != SQLITE_OK)
		return false;
	else
		return true;
}

int Database::putMessage(const char *queueName, const char *message, int messageID)
{
	char *err_msg = NULL;
	char sql[128];
	if (messageID == -1)
		sprintf(sql, "INSERT INTO %s values(NULL, '%s');", queueName, message);
	else
		sprintf(sql, "INSERT INTO %s values(%d, '%s');", queueName, messageID, message);
	if (sqlite3_exec(conn, sql, 0, 0, &err_msg) != SQLITE_OK)
		return -1;
	else
		return sqlite3_last_insert_rowid(conn);
}

string Database::getMessage(const char *queueName, int messageID, bool &ok)
{
	if (!hasMessage(queueName, messageID)) {
		ok = false;
		string error;
		return error;
	}

	char *err_msg = NULL;
	string result;

	char sql[128];
	sprintf(sql, "SELECT * FROM %s WHERE id=%d;", queueName, messageID);
	if (sqlite3_exec(conn, sql, &getMessageCallback, &result, &err_msg) != SQLITE_OK) {
		ok = false;
		string error;
		return error;
	} else {
		ok = true;
		return result;
	}
}

bool Database::deleteMessage(const char *queueName, int messageID)
{
	if (!hasMessage(queueName, messageID))
		return false;

	char *err_msg = NULL;
	char sql[128];
	sprintf(sql, "DELETE FROM [%s] WHERE id=%d;", queueName, messageID);
	if (sqlite3_exec(conn, sql, 0, 0, &err_msg) != SQLITE_OK)
		return false;
	else
		return true;
}

int hasMessageCallback(void *data, int n_columns, char **col_values, char **col_names)
{
	bool *result = (bool *)data;

	*result = true;

	return 0;
}

bool Database::hasMessage(const char *queueName, int messageID)
{
	char *err_msg = NULL;
	bool result;

	char sql[128];
	sprintf(sql, "SELECT * FROM %s WHERE id=%d;", queueName, messageID);
	if (sqlite3_exec(conn, sql, &hasMessageCallback, &result, &err_msg) != SQLITE_OK) {
		return result;
	} else {
		return result;
	}
}

int listQueueCallback(void *data, int n_columns, char **col_values, char **col_names)
{
	vector<string> *lists = (vector<string> *)data;

	string str(col_values[0]);
	if (str == "sqlite_sequence")
		return 0;
	lists->push_back(str);

	return 0;
}

int getMessageCallback(void *data, int n_columns, char **col_values, char **col_names)
{
	string *message = (string *)data;

	*message = col_values[1];

	return 0;
}