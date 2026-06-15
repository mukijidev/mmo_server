#include "Data.h"
#include "TextParser.h"

namespace Data
{
	int ServerConcurrentThreadNum = 0;
	int ServerWorkerThreadNum= 0;
	uint16 ServerPort = 0;
	int SendZeroCopy = -1;
	int Nagle = -1;
	int LogLevel = -1;
	int MonitorClientConcurrentThreadNum = 0;
	int MonitorClientWorkerThreadNum = 0;
	uint16 MonitorPort = 0;
	std::string MonitorServerIp = "";

	int MonitorActivate = -1;
	uint16 DBPort = 0;
	
	uint16 chattingServerPort = 0;
	uint16 loginServerPort = 0;
	uint16 gameServerPort = 0;


	uint16 serverPacketCode = 0;
	uint16 serverPacketKey = 0;
	uint16 clientPacketCode = 0;
	uint16 clientPacketKey = 0;


	bool LoadData()
	{

		TextParser textParser;
		bool loadConfigSucceed = textParser.LoadConfigFile("ServerConfig.txt");
		if (!loadConfigSucceed)
		{
			wprintf(L"loadConfig failed");
			return false;

		}
		bool changeNameSpaceSucceed = textParser.ChangeNamespace("Server");
		if (!changeNameSpaceSucceed)
		{
			wprintf(L"changeNameSpaceSucceed failed");
			return false;
		}
		ServerConcurrentThreadNum = textParser.GetInt("ServerConcurrentThreadNum");
		if (ServerConcurrentThreadNum == 0)
		{
			wprintf(L"load concurrentThreadNum failed");
			return false;
		}

		ServerWorkerThreadNum = textParser.GetInt("ServerWorkerThreadNum");
		if (ServerWorkerThreadNum == 0)
		{
			wprintf(L"load workerThreadNum failed");
			return false;
		}

		ServerPort = textParser.GetInt("ServerPort");
		if (ServerPort == 0)
		{
			wprintf(L"load port failed");
			return false;
		}

		SendZeroCopy = textParser.GetInt("SendZeroCopy");
		if (SendZeroCopy == -1)
		{
			wprintf(L"load SendZeroCopy failed");
			return false;
		}

		Nagle = textParser.GetInt("Nagle");
		if (Nagle == -1)
		{
			wprintf(L"load Nagle failed");
			return false;
		}

		DBPort = textParser.GetInt("DBPort");
		if (DBPort == 0)
		{
			wprintf(L"load DBPort failed");
			return false;
		}

		bool chaangeNameSapceToLog = textParser.ChangeNamespace("Log");
		if (!chaangeNameSapceToLog)
		{
			wprintf(L"chaangeNameSapceToLogl failed");
			return false;

		}

		LogLevel = textParser.GetInt("LogLevel");
		if (LogLevel == -1)
		{
			wprintf(L"load logLevel failed");
			return false;
		}

		bool changeNameSpaceToClient = textParser.ChangeNamespace("MonitorClient");
		if (!changeNameSpaceToClient)
		{
			wprintf(L"changeNameSpaceToClient");
			return false;
		}

		MonitorClientConcurrentThreadNum = textParser.GetInt("MonitorClientConcurrentThreadNum");
		if (MonitorClientConcurrentThreadNum == 0)
		{
			wprintf(L"load ClientWorkerThreadNum failed");
			return false;
		}

		MonitorClientWorkerThreadNum = textParser.GetInt("MonitorClientWorkerThreadNum");
		if (MonitorClientWorkerThreadNum == 0)
		{
			wprintf(L"load ClientWorkerThreadNum failed");
			return false;
		}

		MonitorPort = textParser.GetInt("MonitorPort");
		if (MonitorPort == 0)
		{
			wprintf(L"load MonitorPort failed");
			return false;
		}

		MonitorServerIp = textParser.GetString("MonitorServerIp");
		if (MonitorServerIp == "")
		{
			wprintf(L"load MonitorServerIp failed");
			return false;
		}

		MonitorActivate = textParser.GetInt("MonitorActivate");
		if (MonitorActivate == -1)
		{
			wprintf(L"load MonitorActivate failed");
			return false;
		}

		bool changeNameSpaceToPacket = textParser.ChangeNamespace("Packet");
		if (!changeNameSpaceToPacket)
		{
			wprintf(L"changeNameSpaceToPacket failed");
			return false;
		}

		serverPacketCode = textParser.GetInt("ServerPacketCode");
		if (serverPacketCode == 0)
		{
			wprintf(L"load serverPacketCode failed");
			return false;
		}

		serverPacketKey = textParser.GetInt("ServerPacketKey");
		if (serverPacketKey == 0)
		{
			wprintf(L"load serverPacketKey failed");
			return false;
		}

		clientPacketCode = textParser.GetInt("ClientPacketCode");
		if (clientPacketCode == 0)
		{
			wprintf(L"load clientPacketCode failed");
			return false;
		}

		clientPacketKey = textParser.GetInt("ClientPacketKey");
		if (clientPacketKey == 0)
		{
			wprintf(L"load clientPacketKey failed");
			return false;
		}

		bool changeNameSpaceToChatting = textParser.ChangeNamespace("ChattingServer");
		if (!changeNameSpaceToChatting)
		{
			wprintf(L"changeNameSpaceToChattingServer failed");
			return false;
		}

		chattingServerPort = textParser.GetInt("ChattingServerPort");
		if (chattingServerPort == 0)
		{
			wprintf(L"load chattingServerPort failed");
			return false;
		}

		bool changeNameSpaceToLogin = textParser.ChangeNamespace("LoginServer");
		if (!changeNameSpaceToLogin)
		{
			wprintf(L"changeNameSpaceToLogin failed");
			return false;
		}

		loginServerPort = textParser.GetInt("LoginServerPort");
		if (loginServerPort == 0)
		{
			wprintf(L"load loginServerPort failed");
			return false;
		}

		bool changeNameSpaceToGame = textParser.ChangeNamespace("GameServer");
		if (!changeNameSpaceToGame)
		{
			wprintf(L"changeNameSpaceToGame failed");
			return false;
		}

		gameServerPort = textParser.GetInt("GameServerPort");
		if (gameServerPort == 0)
		{
			wprintf(L"load gameServerPort failed");
			return false;
		}



		return true;
	}
}
