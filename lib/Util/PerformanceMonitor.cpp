#include "PerformanceMonitor.h"
#include <stdio.h>
using namespace std;

PerformanceMonitor::PerformanceMonitor(wstring processName)
{
	PdhOpenQuery(NULL, NULL, &processUserAllocMemoryQuery);
	PdhOpenQuery(NULL, NULL, &availableMemoryQuery);
	PdhOpenQuery(NULL, NULL, &nonPageMemoryQuery);
	//PdhOpenQuery(NULL, NULL, &cpuQuery);
	//PdhOpenQuery(NULL, NULL, &cpuUserQuery);
	//PdhOpenQuery(NULL, NULL, &cpuKernelQuery);
	//PdhOpenQuery(NULL, NULL, &processNonPagedMemoryQuery);
	//PdhOpenQuery(NULL, NULL, &procssCpuQuery);
	WCHAR path[MAX_PATH];
	wsprintf(path, L"\\Process(%s)\\Private Bytes", processName.c_str());

	PdhAddCounter(processUserAllocMemoryQuery, path, NULL, &processUserAllocMemoryTotal);
	PdhAddCounter(availableMemoryQuery, L"\\Memory\\Available MBytes", NULL, &availableMemoryTotal);
	PdhAddCounter(nonPageMemoryQuery, L"\\Memory\\Pool Nonpaged Bytes", NULL, &nonPageMemoryTotal);
	//PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
	//// PDH 리소스 카운터 생성 (여러개 수집시 이를 여러개 생성)
	//PdhAddCounter(cpuUserQuery, L"\\Processor(_Total)\\% User Time", NULL, &cpuUserTotal);
	//PdhAddCounter(procssCpuQuery, L"\\Processor(ChattingServer)\\% Processor Time", NULL, &procssCpuTotal);
	//// PDH 리소스 카운터 생성 (여러개 수집시 이를 여러개 생성)
	//PdhAddCounter(cpuKernelQuery, L"\\Processor(_Total)\\% Privileged Time", NULL, &cpuKernelTotal);
	//PdhAddCounter(processNonPagedMemoryQuery, L"\\Process(ChattingServer)\\Pool Nonpaged Bytes", NULL, &processNonPagedMemoryTotal);
	PdhCollectQueryData(processUserAllocMemoryQuery);
	PdhCollectQueryData(availableMemoryQuery);
	PdhCollectQueryData(nonPageMemoryQuery);
	//PdhCollectQueryData(cpuQuery);
	//PdhCollectQueryData(cpuUserQuery);
	//PdhCollectQueryData(cpuKernelQuery);
	////PdhCollectQueryData(procssCpuQuery);
	//PdhCollectQueryData(processNonPagedMemoryQuery);
}

void PerformanceMonitor::Update(PerformanceMonitorData& performanceMonitorData)
{
	PdhCollectQueryData(processUserAllocMemoryQuery);
	PdhCollectQueryData(availableMemoryQuery);
	PdhCollectQueryData(nonPageMemoryQuery);
	PdhGetFormattedCounterValue(processUserAllocMemoryTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.processUserAllocMemoryCounterVal);
	PdhGetFormattedCounterValue(availableMemoryTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.availableMemoryCounterVal);
	PdhGetFormattedCounterValue(nonPageMemoryTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.nonPageMemoryCounterVal);
	/*
	PdhCollectQueryData(cpuQuery);
	PdhCollectQueryData(cpuUserQuery);
	PdhCollectQueryData(cpuKernelQuery);
	//PdhCollectQueryData(procssCpuQuery);
	PdhCollectQueryData(processNonPagedMemoryQuery);

	PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.cpuCounterVal);
	PdhGetFormattedCounterValue(cpuUserTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.cpuUserCounterVal);
	PdhGetFormattedCounterValue(cpuKernelTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.cpuKernelCounterVal);
	//PdhGetFormattedCounterValue(procssCpuTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.processCpuCounterVal);
	PdhGetFormattedCounterValue(processNonPagedMemoryTotal, PDH_FMT_DOUBLE, NULL, &performanceMonitorData.processNonPagedMemoryCounterval);
	*/
}
