#pragma once
#include "Type.h"
#include <string>


namespace Data
{
	bool LoadData();
	extern int ServerConcurrentThreadNum;
	extern int ServerWorkerThreadNum;
	extern uint16 ServerPort;
	extern int SendZeroCopy;
	extern int Nagle;
	extern int LogLevel;
	extern std::string MonitorServerIp;
	extern int MonitorActivate;
	extern int MonitorClientConcurrentThreadNum;
	extern int MonitorClientWorkerThreadNum;
	extern uint16 MonitorPort;
	extern uint16 DBPort;
	
	extern uint16 chattingServerPort;
	extern uint16 loginServerPort;
	extern uint16 gameServerPort;

	extern std::string gameServerIp;
	extern std::string chatServerIp;


	extern uint16 serverPacketCode;
	extern uint16 serverPacketKey;
	extern uint16 clientPacketCode;
	extern uint16 clientPacketKey;
}
