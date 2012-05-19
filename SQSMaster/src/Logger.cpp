#include "Logger.h"

#include <cstdio>
#include <string>
#include <vector>
#include <cstdlib>

using namespace std;

Logger::Logger(const char *filename) : m_filename(filename)
{
	FILE *file = fopen(m_filename.c_str(), "w+");
	fclose(file);
}

Logger::Logger(const string &filename) : m_filename(filename)
{
	FILE *file = fopen(m_filename.c_str(), "w+");
	fclose(file);
}

Logger::~Logger()
{
}

bool Logger::addLog(Logger::Type type, const string &queue, const string &msg)
{
	char cmd[128];
	switch (type) {
	case Logger::Create:
		sprintf(cmd, "echo 'c %s' >> %s", queue.c_str(), m_filename.c_str());
		break;
	case Logger::Write:
		sprintf(cmd, "echo 'w %s:%s' >> %s", queue.c_str(), msg.c_str(), m_filename.c_str());
		break;
	case Logger::Delete:
		sprintf(cmd, "echo 'd %s:%s' >> %s", queue.c_str(), msg.c_str(), m_filename.c_str());
		break;
	}

	int result = system(cmd);
	return result ? false : true;
}

int Logger::count()
{
	int cnt;
	char cmd[128];
	sprintf(cmd, "wc -l %s", m_filename.c_str());
	FILE *pipe = popen(cmd, "r");
	fscanf(pipe, "%d", &cnt);
	pclose(pipe);
	return cnt;
}

vector<string> Logger::tailList(int n)
{
	vector<string> resultVec;
	char cmd[128];
	sprintf(cmd, "tail -n%d %s", n, m_filename.c_str());
	FILE *pipe = popen(cmd, "r");

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, pipe)) != -1) {
		resultVec.push_back(line);
	}
	free(line);
	//delete line;
	pclose(pipe);
	return resultVec;
}
