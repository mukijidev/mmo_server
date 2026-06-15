#include "Type.h"
#include <stdio.h>
#include "Log.h"
#include "strsafe.h"
#include "time.h"
#include <tchar.h>

//int g_logLevel = dfLOG_LEVEL_SYSTEM;
//WCHAR g_LogBuff[1024];
using namespace std;


wstring _logFolder;
//void Log(WCHAR* string, int logLevel)
//{
//	wprintf(L"[LOG LEVEL %d], %s\n", logLevel, string);
//}

LogLevel _logLevel = LogLevel::System;

CRITICAL_SECTION _cs;

void InitSysLog(wstring logDirName)
{
	_logFolder = logDirName;
	CreateDirectory(_logFolder.c_str(), NULL);
	InitializeCriticalSection(&_cs);
}

void SysLogLevel(LogLevel logLevel)
{
	_logLevel = logLevel;
}

#define MAX_FOLDER_NAME 256
#define MAX_LOG_MESSAGE 1024
void Log(const WCHAR* logType, LogLevel logLevel, const WCHAR* stringFormat, ...)
{
	if (logLevel < _logLevel)
		return;

	EnterCriticalSection(&_cs);

	__time64_t long_time;
	_time64(&long_time);
	struct tm newtime;
	_localtime64_s(&newtime, &long_time);

	WCHAR logFolder[MAX_FOLDER_NAME] = { '\0' };
	HRESULT folderRes = StringCchPrintf(logFolder, MAX_FOLDER_NAME, L"%s/%s_%d_%02d.txt",
		_logFolder.c_str(), logType, newtime.tm_year + 1900, newtime.tm_mon + 1);

	if (FAILED(folderRes))
	{
		wprintf(L"failed to log : %d\n", folderRes);
		LeaveCriticalSection(&_cs);
		return;
	}
	WCHAR logMessage[MAX_LOG_MESSAGE] = { '\0', };

	va_list va;
	va_start(va, stringFormat);
	HRESULT messageRes = StringCchVPrintf(logMessage, MAX_LOG_MESSAGE, stringFormat, va);
	va_end(va);

	if (FAILED(messageRes))
	{
		wprintf(L"failed to log : %d\n", messageRes);
		LeaveCriticalSection(&_cs);
		return;
	}
	logMessage[wcslen(logMessage)] = '\0';

	WCHAR log[MAX_LOG_MESSAGE + 100] = { '\0', };


	HRESULT logRes;
	if (logLevel == LogLevel::Debug)
	{
		logRes = StringCchPrintf(log, MAX_LOG_MESSAGE + 100,
			L"[DEBUG ] [%d.%02d.%02d %02d:%02d:%02d] %s\n",
			newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday,
			newtime.tm_hour, newtime.tm_min, newtime.tm_sec,
			logMessage);
	}
	else if (logLevel == LogLevel::Error)
	{
		logRes = StringCchPrintf(log, MAX_LOG_MESSAGE + 100,
			L"[ERROR ] [%d.%02d.%02d %02d:%02d:%02d] %s\n",
			newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday,
			newtime.tm_hour, newtime.tm_min, newtime.tm_sec,
			logMessage);
	}
	else
	{
		logRes = StringCchPrintf(log, MAX_LOG_MESSAGE + 100,
			L"[SYSTEM] [%d.%02d.%02d %02d:%02d:%02d] %s\n",
			newtime.tm_year + 1900, newtime.tm_mon + 1, newtime.tm_mday,
			newtime.tm_hour, newtime.tm_min, newtime.tm_sec,
			logMessage);
	}



	if (FAILED(logRes))
	{
		wprintf(L"failed to log : %d\n", logRes);
		LeaveCriticalSection(&_cs);
		return;
	}

	FILE* f;
	errno_t err = _wfopen_s(&f, logFolder, L"a+");
	if (err != 0)
	{
		wprintf(L"failed to open fild : %d\n", err);
		LeaveCriticalSection(&_cs);
		return;
	}

	if (f != nullptr)
	{
		fwprintf_s(f, log);
		fclose(f);
	}

	LeaveCriticalSection(&_cs);
}
