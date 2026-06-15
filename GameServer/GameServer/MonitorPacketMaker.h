#pragma once
#include "Type.h"
class CPacket;


void MP_CS_MONITOR_VALUE(CPacket* packet, uint16 sessionNumPerSector[50][50],
	double cpuUsageTotal, double cpuUsageUser, double cpuUsageKernel,
	double processUserAllocMemory, double processNonPageMemory,
	double availableMemory, double nonPagedMemory,
	int64 totalAccept, int64 acceptTps, int64 sendTps, int64 recvTps, int64 sessionNum,
	int64 totalDisconnect, int64 loginTotal, int64 sectorMoveTotal, int64 messageTotal,
	int64 heartbeatTotal, int64 averageRtt, int64 lastRtt);
