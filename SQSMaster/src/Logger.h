#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>

using namespace std;

class Logger
{
public:
	enum Type {Create, Write, Delete};
	Logger(const char *filename);
	Logger(const string &filename);
	virtual ~Logger();

	/**
	 * @brief Add a record to the log file
	 * 
	 * @p type should be one of the enum variable.
	 * @p queue is the queue's name
	 * @p msgID should be: 1. empty when type is Create. 2. the message when type is Write. 3.the msgID when Delete.
	 * 
	 * FIXME: according to the project. DeleteMessage(int64 MessageID) only accept a msgID. should we add a queue name?
	 * 
	 * @return true if success, false otherwise
	 */
	bool addLog(Logger::Type type, const string &queue, const string &msg);
	
	/**
	 * @return the number of recorf in the log file.
	 */
	int count();
	
	/**
	 * @return a vector of the last @p n records.
	 */
	vector<string> tailList(int n);

private:
	string m_filename;

	Logger(const Logger &other);
};

#endif // LOGGER_H
