#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <vector>

class Logger
{
public:
	Logger(const char *filename);
	Logger(const std::string &filename);
	virtual ~Logger();

	/**
	 * @brief Add a record to the log file
	 * 
	 * @p operation the operation url
	 * 
	 * @return true if success, false otherwise
	 */
	bool addLog(const std::string &operation);

	/**
	 * @return the number of recorf in the log file.
	 */
	int count();

	/**
	 * @return a vector of the last @p n records.
	 */
	std::vector<std::string> tailList(int n);

private:
	std::string m_filename;

	Logger(const Logger &other);
};

#endif // LOGGER_H
