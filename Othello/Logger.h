#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <string>
#include "OptimizerConfiguration.h"

class Logger
{
public:
	virtual ~Logger() { }
	virtual void start() = 0;
	virtual void beginEntry() = 0;
	virtual void endEntry() = 0;
	virtual void print(const char *format, ...) = 0;
};

class TxtLogger : public Logger
{
	std::string logFilename;
	FILE *tmpFile;
	bool append;

	void clear()
	{
		if (tmpFile != nullptr)
		{
			fclose(tmpFile);
			tmpFile = nullptr;
		}
	}
public:
	TxtLogger(std::string logFilename, Configuration *configuration, bool append) :
		logFilename(logFilename),
		tmpFile(nullptr),
		append(append)
	{
	}

	~TxtLogger()
	{
		clear();
	}

	void start()
	{
		if (!append)
			fclose(fopen(logFilename.c_str(), "w"));
	}

	void beginEntry()
	{
		clear();
		tmpFile = fopen(logFilename.c_str(), "a");
	}

	void endEntry()
	{
		clear();
	}

	void print(const char *format, ...)
	{
		va_list argptr;
		va_start(argptr, format);
		vfprintf(tmpFile, format, argptr);
		va_end(argptr);
	}
};

#endif // LOGGER_H
