#pragma once
#include "BasePacketHandleThread.h"
#include <map>
#include "LockFreeObjectPool.h"
#include "mysql.h"
#include <vector>
#include "Data.h"
#include "DBSecret.h"

class Player;
class CPacket;
class GameServer;

class LoginGameThread : public BasePacketHandleThread
{
public:
	LoginGameThread(GameServer* gameServer, int threadId, int msPerFrame);
	~LoginGameThread();

private:
	void HandleLogin(Player* player, CPacket* packet);
	void HandleFieldMove(Player* player, CPacket* packet);
	void HandleSignUp(Player* player, CPacket* packet);
	void HandleRequestPlayerList(Player* player, CPacket* packet);
	void HandleSelectPlayer(Player* player, CPacket* packet);
	void HandleCreatePlayer(Player* player, CPacket* packet);

	// BasePacketHandleThread을(를) 통해 상속됨
	void OnLeaveThread(int64 sessionId, bool disconnect) override;
	void OnEnterThread(int64 sessionId, void* ptr) override;

private:
	//mysql
	//TODO: 비밀번호 ip config로 뺴기
	MYSQL _conn;
	MYSQL* _connection;
	const char* host = "127.0.0.1";
	const char* user = "root";
	const char* password = DB_PASSWORD;
	const char* database = "mmo";
	int port = Data::DBPort;


	int64 lastPlayerId = 0;
	// GameThread을(를) 통해 상속됨
	void GameRun(float deltaTime) override;

private:
	// BasePacketHandleThread을(를) 통해 상속됨
	void HandleAsyncJobFinish(void* ptr, uint16 jobType) override;

};

