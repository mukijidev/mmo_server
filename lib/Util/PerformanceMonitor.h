#pragma once
#include "Pdh.h"
#pragma comment(lib,"Pdh.lib")
#include <string>


struct PerformanceMonitorData
{
	PDH_FMT_COUNTERVALUE processUserAllocMemoryCounterVal;
	PDH_FMT_COUNTERVALUE availableMemoryCounterVal;
	PDH_FMT_COUNTERVALUE nonPageMemoryCounterVal;

	//PDH_FMT_COUNTERVALUE cpuCounterVal;
	//PDH_FMT_COUNTERVALUE cpuUserCounterVal;
	//PDH_FMT_COUNTERVALUE cpuKernelCounterVal;
	//PDH_FMT_COUNTERVALUE processCpuCounterVal;
	//PDH_FMT_COUNTERVALUE processNonPagedMemoryCounterval;
};

class PerformanceMonitor
{
public:
	PerformanceMonitor(std::wstring processName);
	void Update(PerformanceMonitorData& performanceMonitorData);

public:
	PDH_HQUERY processUserAllocMemoryQuery;
	PDH_HQUERY availableMemoryQuery;
	PDH_HQUERY nonPageMemoryQuery;

	//PDH_HQUERY processNonPagedMemoryQuery;
	//PDH_HQUERY cpuQuery;
	//PDH_HQUERY cpuUserQuery;
	//PDH_HQUERY cpuKernelQuery;
	//PDH_HQUERY procssCpuQuery;
public:
	PDH_HCOUNTER processUserAllocMemoryTotal;
	PDH_HCOUNTER availableMemoryTotal;
	PDH_HCOUNTER nonPageMemoryTotal;

	//PDH_HCOUNTER processNonPagedMemoryTotal;

	//PDH_HCOUNTER cpuTotal;
	//PDH_HCOUNTER cpuUserTotal;
	//PDH_HCOUNTER cpuKernelTotal;
	//PDH_HCOUNTER procssCpuTotal;
};

