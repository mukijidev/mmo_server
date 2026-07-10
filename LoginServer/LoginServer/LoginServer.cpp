#define _CRT_SECURE_NO_WARNINGS
#define _CRT_RAND_S

#include "LoginServer.h"
#include "SerializeBuffer.h"
#include "PacketHeader.h"
#include <process.h>
#include "TlsObjectPool.h"
#include "Packet.h"
#include "Profiler.h"
#include "LockFreeObjectPool.h"
#include <inttypes.h>
#include "MonitorPacketMaker.h"
#include "Log.h"
#include <algorithm>
#include <cpp_redis/cpp_redis>
#include "CpuUsage.h"
#include <Type.h>
#include "MonitorProtocol.h"

using namespace std;

LoginServer::LoginServer()
{
	//섹터 초기화
	// Player Array 초기화
	for (int i = 0; i < MAX_PLAYER_NUM; i++)
	{
		// 후에 stack에 사용될 index 설정
		_playerArr[i]._index = i;
		// init lock
		InitializeSRWLock(&_playerArr[i]._lock);
	}

	// MAX_PLAYER_NUM까지 사용 가능 player stack에 넣고
	for (int i = MAX_PLAYER_NUM - 1; i >= 0; i--)
	{
		_emptyPlayerIndex.Push(i);
	}

	// playerMap lock 초기화
	InitializeSRWLock(&_playerMapLock);
	int errorCode;

	_hTimeOutThread = (HANDLE)_beginthreadex(NULL, 0, TimeOutThreadStatic, this, 0, NULL);
	if (_hTimeOutThread == NULL)
	{
		errorCode = WSAGetLastError();
		LOG(L"System", LogLevel::System, L"Start TimeOut Thread Error");
		__debugbreak();
	}

	InitMySQLConnection();
	InitRedisConnection();

	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, MonitorThreadStatic, this, 0, NULL);
	if (_hMonitorThread == NULL)
	{
		errorCode = WSAGetLastError();
		__debugbreak();
	}

	LOG(L"System", LogLevel::System, L"LoginServer()");
}


LoginServer::~LoginServer()
{

	WaitForSingleObject(_hTimeOutThread, INFINITE);
	//모니터 클라 켜진상태면	
	if (_monitorActivated)
	{
		WaitForSingleObject(_hMonitorSendThread, INFINITE);
	}
	ClearMySQLonneection();
	ClearRedisConnection();

	LOG(L"System", LogLevel::System, L"~LoginServer()");
}

bool LoginServer::OnConnectionRequest()
{
	//virtual bool OnConnectionRequest(IP, Port) = 0; < accept 직후
	/*return false; 시 클라이언트 거부.
	return true; 시 접속 허용*/

	//사용처 : emptyPlayerIndex 비어있을시 return false;
	//아직은 호출 안함
	if (_emptyPlayerIndex.size() == 0)
	{
		return false;
	}

	return true;
}

//void LoginServer::OnAccept(int64 sessionId)
//{
//
//}

int64 LoginServer::LoginByIdPassword(const char* id, const char* pw)
{
	MYSQL_STMT* stmt = mysql_stmt_init(_connections[_connectionIndex]);
	const char* selectQuery = "SELECT AccountNo FROM Account WHERE ID = ? AND PassWord = ?";
	mysql_stmt_prepare(stmt, selectQuery, (unsigned long)strlen(selectQuery));

	MYSQL_BIND param[2] = {};
	param[0].buffer_type = MYSQL_TYPE_STRING;
	param[0].buffer = (void*)id;
	param[0].buffer_length = (unsigned long)strlen(id);
	param[1].buffer_type = MYSQL_TYPE_STRING;
	param[1].buffer = (void*)pw;
	param[1].buffer_length = (unsigned long)strlen(pw);
	mysql_stmt_bind_param(stmt, param);
	mysql_stmt_execute(stmt);
	
	int64 accountNo;
	bool isNull = false;
	MYSQL_BIND result[1] = {};
	result[0].buffer_type = MYSQL_TYPE_LONGLONG;
	result[0].buffer = &accountNo;
	result[0].is_null = &isNull;
	mysql_stmt_bind_result(stmt, result);
	mysql_stmt_store_result(stmt);

	int64 retVal = -1;
	if (mysql_stmt_fetch(stmt) == 0 && !isNull)
		retVal = accountNo;

	mysql_stmt_close(stmt);
	return retVal;
}

std::string LoginServer::GenerateSessionKey()
{
	unsigned char buf[SESSION_KEY_LEN / 2];
	for (int i = 0; i < SESSION_KEY_LEN / 2; i++)
	{
		unsigned int r;
		rand_s(&r);
		buf[i] = (unsigned char)r;
	}

	const char* hx = "0123456789abcdef";
	string sessionKey;
	sessionKey.reserve(SESSION_KEY_LEN);
	for (unsigned char c : buf)
	{
		sessionKey.push_back(hx[c >> 4]);
		sessionKey.push_back(hx[c & 0xF]);
	}

	return sessionKey;
}


void LoginServer::OnAccept(int64 sessionId, WCHAR* ip)
{
	printf("accept\n");
	LOG(L"Develop", LogLevel::Debug, L"Accept Client");
	
	PRO_BEGIN(L"OnAccept");
	uint16 playerIndex = MAX_PLAYER_NUM;
	bool popSucceed = _emptyPlayerIndex.Pop(playerIndex);
	if (!popSucceed)
	{
		//MAX_PLAYER_NUM 넘치게 player되는지 확인 ( 에러 )
		LOG(L"Accept", LogLevel::Error, L"Pop Empty Player Index Failed");
		__debugbreak();
		// 넘치면 끊고
		Disconnect(sessionId); 
	}

	//플레이어 초기화 해주고
	Player* player = &_playerArr[playerIndex];
	AcquireSRWLockExclusive(&player->_lock);
	player->Init(sessionId, ip);
	ReleaseSRWLockExclusive(&player->_lock);

	//맵에 넣어주고
	AcquireSRWLockExclusive(&_playerMapLock);
	_playerMap.insert({ sessionId, player });
	ReleaseSRWLockExclusive(&_playerMapLock);
	PRO_END(L"OnAccept");
}

void LoginServer::OnDisconnect(int64 sessionId)
{
	printf("disconnect\n");
	LOG(L"Develop", LogLevel::Debug, L"Disconnect Client");
	PRO_BEGIN(L"OnDisconnect");
	Player* player = nullptr;
	//맵에서 먼저 삭제하고 index
	AcquireSRWLockExclusive(&_playerMapLock);
	auto it = _playerMap.find(sessionId);
	if (it == _playerMap.end())
	{
		__debugbreak();
	}
	player = (*it).second;
	_playerMap.erase(it);
	ReleaseSRWLockExclusive(&_playerMapLock);
	

	AcquireSRWLockExclusive(&player->_lock);
	// 1. 세션 끊어져서 OnDisconnect왔음
	// 2. player lock 잡고 삭제하고 sessionId -1로 바꾸면
	// 3. OnRecvPacket에서 player찾은 상태로 있다가 playerLock을 잡고 sessionId 확인하면 -1이기 떄문에
	// 4. 잘못된 플레이어 사용 막을 수 있믐
	player->_sessionId = -1; 
	ReleaseSRWLockExclusive(&player->_lock);
	
	// 재사용 할 수 있게 player Index 넣고
	_emptyPlayerIndex.Push(player->_index);
	PRO_END(L"OnDisconnect");
}


void LoginServer::OnRecvPacket(int64 sessionId, CPacket* packet)
{
	// 플레이어 찾아서
	// 플레아어의 packetQueue에넣기
	Player* player;
	AcquireSRWLockShared(&_playerMapLock);
	auto it = _playerMap.find(sessionId);
	if (it == _playerMap.end())
	{
		__debugbreak();
	}
	player = (*it).second;
	ReleaseSRWLockShared(&_playerMapLock);
	
	//player를 찾았는데 lock을 잡았더니 sessionId가 바뀌었으면
	//이미 끊어진 세션이다
	AcquireSRWLockExclusive(&player->_lock);
	if (sessionId != player->_sessionId)
	{
		// 이 경우 생기는지 확인
		// 더미가 요청에 대한 응답이 오고나서야 요청을 보내기 때문에 이 경우가 안생기는게 맞는데
		// 원장님 채팅 더미 말고 일반적인 상황에서는 생길 수 있다.
		InterlockedIncrement64(&_playerReuseCount);
		ReleaseSRWLockExclusive(&player->_lock);
		return;
	}

	PRO_BEGIN(L"Handle");
	HandleRecvPacket(player, packet);
	PRO_END(L"Handle");
	ReleaseSRWLockExclusive(&player->_lock);
}

void LoginServer::HandleRecvPacket(Player* player, CPacket* packet)
{
	uint16 packetType;
	*packet >> packetType;

	switch (packetType)
	{

	case PACKET_CS_LOGIN_REQ_LOGIN:
	{
		LOG(L"Develop", LogLevel::Debug, L"Login Recv");

		// 로그인  메시지
		InterlockedIncrement64(&_loginTotal);
		PRO_BEGIN(L"HandleLogin");
		HandleLogin(player, packet);
		PRO_END(L"HandleLogin");
	}
	break;

	case PACKET_CS_LOGIN_REQ_ECHO:
	{
		LOG(L"Develop", LogLevel::Debug, L"Echo Recv");
		HandleEcho(player, packet);
	}
	break;

	case PACKET_CS_LOGIN_REQ_SIGN_UP:
	{
		HandleSignUp(player, packet);
	}
	break;

	default:
		LOG(L"Packet", LogLevel::Error, L"Packet Type Not Exist");
		__debugbreak();//TODO: 악의적인 유저가 이상한 패킷을 보냈다-> 세션 끊는다
	}

	CPacket::Free(packet);
}


bool LoginServer::GetUserDataFromMysql(Player* p)
{
	//const char* query = "SELECT user_id, user_nick FROM account WHERE accountno : %d";	// From 다음 DB에 존재하는 테이블 명으로 수정하세요
	
	int query_stat;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	char query[100];
	sprintf(query, "SELECT userid, usernick FROM account WHERE accountno = %lld", p->accountNo);

	query_stat = mysql_query(_connections[_connectionIndex], query);
	if (query_stat != 0)
	{
		printf("Mysql query error : %s", mysql_error(&_conn[_connectionIndex]));
		return false;
	}

	sql_result = mysql_store_result(_connections[_connectionIndex]);		// 결과 전체를 미리 가져옴

	
	sql_row = mysql_fetch_row(sql_result);
	if (sql_row == NULL)
	{
		return false;
	}

	memcpy(p->id, sql_row[0], strlen(sql_row[0]));
	memcpy(p->nickName, sql_row[1], strlen(sql_row[1]));

		mysql_free_result(sql_result);
	return true;
}

void LoginServer::SetUserDataToRedis(Player* p)
{
	__int64 accountNo = p->accountNo;
	string key = std::to_string(p->accountNo);
	//_redis->setex(key, dfREDIS_TIMEOUT, user->_sessionKey);
	_redisClients[_connectionIndex].set(key, p->_sessionKey);
	_redisClients[_connectionIndex].sync_commit();

}

void LoginServer::GetIpForPlayer(Player* p, WCHAR* gameServerIp, uint16& gameServerPort, WCHAR* chatServerIp, uint16& chatServerPort)
{

	std::wstring wGameIp(Data::gameServerIp.begin(), Data::gameServerIp.end());
	std::wstring wChatIp(Data::chatServerIp.begin(), Data::chatServerIp.end());

	wmemcpy(gameServerIp, wGameIp.c_str(), wGameIp.size());
	gameServerIp[wGameIp.size()] = L'\0';
	wmemcpy(chatServerIp, wChatIp.c_str(), wChatIp.size());
	chatServerIp[wChatIp.size()] = L'\0';

	gameServerPort = _gameServerPort;
	chatServerPort = _chattingServerPort;



}


void LoginServer::HandleLogin(Player* player, CPacket* packet)
{
	InterlockedIncrement(&_loginTps);
	if (_connectionIndex == -1)
	{
		_connectionIndex = InterlockedIncrement(&connectionIndexCounter);
	}
	player->_lastRecvTime = timeGetTime();
	
	WCHAR wid[dfID_LEN];
	WCHAR wpw[dfPASSWORD_LEN];
	packet->GetData((char*)wid, dfID_LEN * sizeof(WCHAR));
	packet->GetData((char*)wpw, dfPASSWORD_LEN * sizeof(WCHAR));
	char id[ID_LEN * sizeof(WCHAR)];
	char password[ID_LEN * sizeof(WCHAR)];

	wcstombs(id, wid, dfID_LEN * sizeof(WCHAR));
	wcstombs(password, wpw, dfPASSWORD_LEN * sizeof(WCHAR));

	uint8 status = 0;
	int64 accountNo = LoginByIdPassword(id, password);
	
	if (accountNo != -1)
		status = 1;

	// 로그인 실패
	if (status == 0)
	{
		CPacket* resPacket = CPacket::Alloc();
		WCHAR ip[IP_LEN] = L"";
		uint16 zero = 0;
		MP_SC_LOGIN(resPacket, accountNo, status, ip, zero, ip, zero, nullptr);
		SendPacket_Unicast(player, resPacket);
		CPacket::Free(resPacket);
		return;
	}
	
	player->accountNo = accountNo;
	player->_bLogined = true;

	player->_sessionKey = GenerateSessionKey();

	//redis;
	string key = std::to_string(player->accountNo);
	_redisClients[_connectionIndex].setex(key, dfREDIS_TIMEOUT, player->_sessionKey);
	_redisClients[_connectionIndex].sync_commit();



	WCHAR gameServerIP[IP_LEN];
	uint16 gameServerPort;

	WCHAR chatServerIP[IP_LEN];
	uint16 chatServerPort;

	GetIpForPlayer(player, gameServerIP, gameServerPort, chatServerIP, chatServerPort);

	CPacket* resPacket = CPacket::Alloc();
	MP_SC_LOGIN(resPacket, player->accountNo, status, gameServerIP, gameServerPort, chatServerIP, chatServerPort, player->_sessionKey.c_str());
	SendPacket_Unicast(player, resPacket);
	//printf("send login packet\n");

	CPacket::Free(resPacket);
}


void LoginServer::HandleSignUp(Player* player, CPacket* packet)
{
	if (_connectionIndex == -1)
	{
		_connectionIndex = InterlockedIncrement(&connectionIndexCounter);
	}

	WCHAR wid[dfID_LEN];
	WCHAR wpw[dfPASSWORD_LEN];
	packet->GetData((char*)wid, dfID_LEN * sizeof(WCHAR));
	packet->GetData((char*)wpw, dfPASSWORD_LEN * sizeof(WCHAR));
	char id[ID_LEN * sizeof(WCHAR)];
	char password[ID_LEN * sizeof(WCHAR)];

	wcstombs(id, wid, dfID_LEN * sizeof(WCHAR));
	wcstombs(password, wpw, dfPASSWORD_LEN * sizeof(WCHAR));

	//insert
	MYSQL_STMT* stmt = mysql_stmt_init(_connections[_connectionIndex]);
	const char* sql = "INSERT INTO Account(ID, PassWord) VALUES(?, ?)";
	mysql_stmt_prepare(stmt, sql, (unsigned long)strlen(sql));
	MYSQL_BIND b[2] = {};
	b[0].buffer_type = MYSQL_TYPE_STRING;
	b[0].buffer = id;
	b[0].buffer_length = (unsigned long)strlen(id);
	b[1].buffer_type = MYSQL_TYPE_STRING;
	b[1].buffer = password;
	b[1].buffer_length = (unsigned long)strlen(password);
	mysql_stmt_bind_param(stmt, b);

	uint8 status = 1;
	if (mysql_stmt_execute(stmt))
	{
		uint32 err = mysql_stmt_errno(stmt);
		if (err != 1062)
		{
			fprintf(stderr, "쿼리 실행 실패: %s\n", mysql_stmt_error(stmt));
			__debugbreak();
		}
		
		status = 0;
	}

	mysql_stmt_close(stmt);

	CPacket* res = CPacket::Alloc();
	MP_SC_SIGN_UP(res, status);
	SendPacket_Unicast(player, res);
	CPacket::Free(res);
}


void LoginServer::HandleEcho(Player* player, CPacket* packet)
{
	CPacket* resPacket = CPacket::Alloc();
	MP_SC_ECHO(resPacket);
	SendPacket_Unicast(player, resPacket);
	//printf("send echo packet\n");
	CPacket::Free(resPacket);

}


unsigned int __stdcall LoginServer::TimeOutThread()
{
	// PLAYER 배열을 한번에 다 검사안하고 나눠서 검사함으로써 timeout player가 많을시에
	// Map lock 오래 거는것 방지
	int order = 0;
	const int devidedNum = 4;
	const int loopTime = TIMEOUT_DISCONNECT / devidedNum;
	const int checkPlayerNumPerLoop = MAX_PLAYER_NUM / devidedNum;
	while (!isNetworkStop())
	{
		int checkNow = (order++) % devidedNum;
		Sleep(loopTime);
		//	uint32 timeNow = timeGetTime();
		//	for (int i = checkNow * checkPlayerNumPerLoop; i < (checkNow + 1) * checkPlayerNumPerLoop; i++)
		//	{
		//		Player* p = &_playerArr[i];
		//		AcquireSRWLockExclusive(&p->_lock);
		//		if (_playerArr[i]._sessionId <= 0)
		//		{
		//			ReleaseSRWLockExclusive(&p->_lock);
		//			continue;
		//		}

		//		uint32 playerLastRecvTime = p->_lastRecvTime;
		//		if (timeNow - playerLastRecvTime > TIMEOUT_DISCONNECT && timeNow > playerLastRecvTime)
		//		{
		//			//__debugbreak();
		//			_timeOutPlayerNum++;
		//			Disconnect(p->_sessionId);
		//		}
		//		ReleaseSRWLockExclusive(&p->_lock);
		//	}
		//}
	}
	return 0;
}


void LoginServer::SendPacket_Unicast(Player* p, CPacket* packet)
{
	SendPacket(p->_sessionId, packet);
}



void LoginServer::LogServerInfo()
{
	int64 totalAcceptValue = GetTotalAcceptValue();
	int64 acceptTPSValue = GetAcceptTPS();
	int64 sendMessageTPSValue = GetSendMessageTPS();
	int64 recvMessageTPSValue = GetRecvMessageTPS();
	int64 totalDisconnect = GetTotalDisconnect();
	int64 sessionNum = GetSessionNum();
	int64 disconnectBySendQueueFull = GetDisconnectSessionNumBySendQueueFull();
	int processUserAllocMemory = _processUserAllocMemory;
	int loginTps = GetLoginTps();
	int packetUseCount = (int)CPacket::GetUseCount();
	int networkSend = (int)GetNetworkSendByteTps() / 1000;
	int networkRecv = (int)GetNetworkRecvByteTps() / 1000;
	printf("Total Accept : %lld\n\
Accept TPS : % lld\n\
Send Message TPS : %lld\n\
Recv Message TPS : %lld\n\
session num :%lld\n\
total disconnect : %lld\n\
disconnect by sendqueue full : %lld\n\
memory : %d\n\
loginTps : %d\n\
packetuseCount : %d\n\
networkSend : %d\n\
networkRecv : %d\n\
================\n\
", totalAcceptValue, acceptTPSValue, sendMessageTPSValue, recvMessageTPSValue, sessionNum,
totalDisconnect, disconnectBySendQueueFull, processUserAllocMemory, loginTps, packetUseCount, networkSend, networkRecv);
}

void LoginServer::ObjectPoolLog()
{
	for (int i = 0; i < MAX_TLS_LOG_INDEX; i++)
	{
		if (_objectPoolMonitor[i].threadId == 0)
			break;

		TlsLog& tlsLog = _objectPoolMonitor[i];
		printf("thread id : %d , size = %d, mallocCount : %d\n", tlsLog.threadId, tlsLog.size, tlsLog.mallocCount);
	}

	printf("stack size : %d\n", releaseStackSize);
}

void LoginServer::OnError(int errorCode, WCHAR* errorMessage)
{
	//TODO: 라이브러리에서 에러생겼을시 받아서 할거 하기
}




bool LoginServer::ActivateMonitorClient(const WCHAR* serverIp, uint16 port, uint32 concurrentThreadNum, uint32 workerThreadNum)
{
	_monitorClient = new MonitorClient();
	// iocp 생성
	bool bMonitorStartSucceed = _monitorClient->Start(serverIp, port, concurrentThreadNum, workerThreadNum);
	if (!bMonitorStartSucceed)
		return false;
	// 연결
	bool bConnectSucceed = _monitorClient->Connect();
	if (!bConnectSucceed)
		return false;

	int errorCode;
	_hMonitorSendThread = (HANDLE)_beginthreadex(NULL, 0, MonitorSendThreadStatic, this, 0, NULL);
	if (_hMonitorSendThread == NULL)
	{
		errorCode = WSAGetLastError();
		LOG(L"System", LogLevel::System, L"Create Monitor Send Thread Failed");
		__debugbreak();
	}

	_monitorActivated = true;
	return true;
}



unsigned int __stdcall LoginServer::MonitorSendThread()
{
	PerformanceMonitorData performanceMonitorData;
	CpuUsage cpuUsage;
	
	CPacket* loginPacket = CPacket::Alloc();
	int serverNo = SERVER_NO_LOGIN;
	MP_SS_MONITOR_LOGIN(loginPacket, serverNo);
	_monitorClient->SendPacket(loginPacket);
	CPacket::Free(loginPacket);

	Sleep(1000);
	while (!isNetworkStop())
	{
		//1초마다 한번씩 보내기 좀 더 늘릴까?
		Sleep(1000);

		long long llTick; time(&llTick); int ts = (int)llTick;
		int packetUseCount = (int)CPacket::GetUseCount();
		_performanceMonitor.Update(performanceMonitorData);
		cpuUsage.UpdateCpuTime();

		SendMonitorData(dfMONITOR_DATA_TYPE_LOGIN_SERVER_RUN, 1, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_LOGIN_SERVER_CPU, (int)cpuUsage.ProcessTotal(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_LOGIN_SERVER_MEM, (int)(performanceMonitorData.processUserAllocMemoryCounterVal.doubleValue / (1024.0 * 1024.0)), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_LOGIN_PACKET_POOL, packetUseCount, ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_LOGIN_SESSION, (int)GetSessionNum(), ts);
		SendMonitorData(dfMONITOR_DATA_TYPE_LOGIN_AUTH_TPS, (int)GetLoginTps(), ts);
	}

	return 0;
}

void LoginServer::InitMySQLConnection()
{
	for (int i = 0; i < MAX_CONNECTION_NUM; i++) {
		mysql_init(&_conn[i]);
		_connections[i] = mysql_real_connect(&_conn[i], host, user, password, database, port, NULL, 0);
		if (_connections[i] == NULL) {
			fprintf(stderr, "Mysql connection error (%d): %s\n", i, mysql_error(&_conn[i]));
			__debugbreak();
		}
	}

	LOG(L"System", LogLevel::System, L"InitMySQLConnection()");
}

void LoginServer::ClearMySQLonneection()
{
	for (int i = 0; i < MAX_CONNECTION_NUM; i++) {
		if (_connections[i] != NULL) {
			mysql_close(_connections[i]);
			printf("Connection %d closed.\n", i);
		}
	}
}

void LoginServer::InitRedisConnection()
{
	for (int i = 0; i < MAX_CONNECTION_NUM; ++i) {
		_redisClients[i].connect();

		if (_redisClients[i].is_connected()) {
		}
		else {
			__debugbreak();
		}
	}
	LOG(L"System", LogLevel::System, L"InitRedisConnection()");

}

void LoginServer::ClearRedisConnection()
{
	for (int i = 0; i < MAX_CONNECTION_NUM; ++i) {
		_redisClients[i].disconnect();
	}
}

unsigned int __stdcall LoginServer::MonitorThread()
{
	int tpsIndex = 0;
	while (true)
	{
		Sleep(1000);
		_loginTpsArr[tpsIndex % TPS_ARR_NUM] = _loginTps;
		_loginTps = 0;
		tpsIndex++;
	}

	return 0;
}

