#include "Logger.h"

#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>

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

bool Logger::addLog(const string& operation)
{
	char cmd[128];
	sprintf(cmd, "echo '%s' >> %s", operation.c_str(), m_filename.c_str());

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
	pclose(pipe);
	return resultVec;
}
