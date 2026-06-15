#pragma once
#include "CNetServer.h"
#include "LockFreeQueue.h"
#include "Player.h"
#include <unordered_map>
#include "MonitorClient.h"
#include "PerformanceMonitor.h"
#include <vector>
#include "mysql.h"
#include "errmsg.h"
#include <cpp_redis/cpp_redis>
#include "Data.h"
#include "DBSecret.h"

#define MAX_PLAYER_NUM 10000
#define MAX_CONNECTION_NUM 50

class LoginServer : public CNetServer
{
public:
	LoginServer();
	~LoginServer();

private:
	void SendPacket_Unicast(Player* p, CPacket* packet);
	void HandleLogin(Player* player, CPacket* packet);
	void HandleEcho(Player* player, CPacket* packet);
	void HandleRecvPacket(Player* player, CPacket* packet);

	bool GetUserDataFromMysql(Player* p);
	void SetUserDataToRedis(Player* p);
	void GetIpForPlayer(Player* p, WCHAR* gameServerIp, uint16& gameServerPort, WCHAR* chatServerIp, uint16& chatServerPort);


private:
	// CLanServer을(를) 통해 상속됨
	virtual bool OnConnectionRequest() override;
	virtual void OnAccept(int64 sessionId, WCHAR* _sessionIp) override;
	virtual void OnDisconnect(int64 sessionId) override;
	virtual void OnRecvPacket(int64 sessionId, CPacket* packet) override;
	virtual void OnError(int errorCode, WCHAR* errorMessage) override;


private: // 타임아웃 쓰레드
	HANDLE _hTimeOutThread;
	static unsigned int __stdcall TimeOutThreadStatic(void* param)
	{
		LoginServer* thisClass = (LoginServer*)param;
		return thisClass->TimeOutThread();
	}
	unsigned int __stdcall TimeOutThread();



public: // 콘솔 로깅
	void LogServerInfo();
	void ObjectPoolLog();
	//void PlayerMapLog();


private: //플레이어 배열
	SRWLOCK _playerMapLock;
	Player _playerArr[MAX_PLAYER_NUM];
	LockFreeStack<uint16> _emptyPlayerIndex;
	std::unordered_map<int64, Player*> _playerMap;



public: //모니터 클라이언트
	bool ActivateMonitorClient(const WCHAR* serverIp, uint16 port, uint32 concurrentThreadNum, uint32 workerThreadNum);
	
private:
	MonitorClient* _monitorClient = nullptr;

	static unsigned int __stdcall MonitorSendThreadStatic(void* arg)
	{
		LoginServer* pThis = (LoginServer*)arg;
		return pThis->MonitorSendThread();
	}

	unsigned int __stdcall MonitorSendThread();



	static unsigned int __stdcall MonitorThreadStatic(void* arg)
	{
		LoginServer* pThis = (LoginServer*)arg;
		return pThis->MonitorThread();
	}

	unsigned int __stdcall MonitorThread();

	HANDLE _hMonitorSendThread;
	bool _monitorActivated = false;
	HANDLE _hMonitorThread;


private: // 모니터 서버로 보낼거 (서버)
	int64 _loginTotal  = 0;
	int64 _sectorMoveTotal = 0;
	int64 _messageTotal = 0;
	int64 _heartBeatTotal = 0;

private:
	int64 _timeOutPlayerNum = 0;
	int _processUserAllocMemory;

	int GetLoginTps()
	{
		static int sendIndex = 0;
		return _loginTpsArr[sendIndex++ % TPS_ARR_NUM];
	}

	int _loginTpsArr[TPS_ARR_NUM] = { 0, };
	long _loginTps = 0;


private: // 모니터 서버로 보낼거 (하드웨어)
	PerformanceMonitor _performanceMonitor{ L"LoginServer" };

private: //점검용
	int64 _playerReuseCount = 0;

private:
	// mysql connection
	// redis connection
	inline static thread_local long _connectionIndex = -1;
	long connectionIndexCounter = -1;

	MYSQL _conn[MAX_CONNECTION_NUM];
	MYSQL* _connections[MAX_CONNECTION_NUM];
	const char* host = "127.0.0.1";
	const char* user = "root";
	const char* password = DB_PASSWORD;
	const char* database = "accountdb";
	int port = Data::DBPort;

	cpp_redis::client _redisClients[MAX_CONNECTION_NUM];

private:
	void InitMySQLConnection();
	void ClearMySQLonneection();
	void InitRedisConnection();
	void ClearRedisConnection();

	uint16 _chattingServerPort = Data::chattingServerPort;
	uint16 _gameServerPort = Data::gameServerPort;

private:
	uint16 serverPacketCode = Data::serverPacketCode;
	uint16 clientPacketCode = Data::clientPacketCode;

private: // client
	void MP_SC_MONITOR_TOOL_DATA_UPDATE(CPacket* packet, uint8& dataType, int& dataValue, int& timeStamp);
	void MP_SS_MONITOR_LOGIN(CPacket* packet, int& serverNo);

private: // server
	void MP_SC_LOGIN(CPacket* packet, int64& accountNo, uint8& status, WCHAR* gameServerIP, uint16& gameServerPort, WCHAR* chatServerIP, uint16& chatServerPort);
	void MP_SC_ECHO(CPacket* packet);

	// CNetServer을(를) 통해 상속됨
	void OnAccept(int64 sessionId) override;
};

