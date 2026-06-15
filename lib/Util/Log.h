#pragma once
#include <stdio.h>
#include "Type.h"
#include <string>
#define LOG_LEVEL_DEBUG		0
#define LOG_LEVEL_ERROR		1
#define LOG_LEVEL_SYSTEM	2

enum LogLevel {
	Debug = 0,
	Error = 1,
	System = 2
};

void InitSysLog(std::wstring logDirName);
void SysLogLevel(LogLevel logLevel);
void Log(const WCHAR* logType, LogLevel logLevel, const WCHAR* stringFormat, ...);

#define INIT_SYSLOG(logDirName)		InitSysLog(logDirName)
#define SYSLOG_LEVEL(logLevel)				SysLogLevel(logLevel)
#define LOG(logType, logLevel, stringFormat, ...) Log(logType, logLevel, stringFormat, ##__VA_ARGS__)

