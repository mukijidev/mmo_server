#pragma once
#pragma pack (push, 1)
struct MONITOR_PACKET
{
	uint16 type;
	uint16 sessionNmPerSector[50][50];
	double cpuUsageTotal;
	double cpuUsageUser;
	double cpuUsageKernel;
	double processUserAllocMemory;
	double processNonPagedMemory;
	double nonPagedMemory;
};
#pragma pack(pop)