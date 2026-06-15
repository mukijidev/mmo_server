#pragma once

#include "Type.h"


//#define PACKET_CODE 0x77
//#define PACKET_KEY 0x32

#define IP_LEN 16
#define ID_LEN 20
#define NICKNAME_LEN 20
#define SESSION_KEY_LEN 64
#define TIMEOUT_DISCONNECT 40000
#define LOGIN_SERVER_NO 1

//#ifndef __GODDAMNBUG_ONLINE_PROTOCOL__
//#define __GODDAMNBUG_ONLINE_PROTOCOL__



#define dfID_LEN 20
#define dfPASSWORD_LEN 20

enum PACKET_TYPE
{
	//------------------------------------------------------
	// Login Server
	//------------------------------------------------------
	PACKET_LOGIN_SERVER = 0,

	//------------------------------------------------------------
	// 로그인 서버로 클라이언트 로그인 요청
	//
	//	{
	//		WORD	Type
	//
	//		TCHAR   ID[20]
	//		TCHAR	PassWord[20]     //사용자 PassWord. null포함
	//	}
	//------------------------------------------------------------
	PACKET_CS_LOGIN_REQ_LOGIN,

	//------------------------------------------------------------
	// 로그인 서버에서 클라이언트로 로그인 응답
	//
	//	{
	//		WORD	Type
	//
	//		INT64	AccountNo
	//		BYTE	Status				// 0 (세션오류) / 1 (성공) ...  하단 defines 사용
	//
	// 
	//		WCHAR	GameServerIP[16]	// 접속대상 게임,채팅 서버 정보
	//		USHORT	GameServerPort
	//		WCHAR	ChatServerIP[16]
	//		USHORT	ChatServerPort
	//	}
	//
	//------------------------------------------------------------
	PACKET_SC_LOGIN_RES_LOGIN,


	//------------------------------------------------------------
	//
	//	{
	//		WORD	Type
	//	}
	//------------------------------------------------------------
	PACKET_CS_LOGIN_REQ_ECHO,

	//------------------------------------------------------------
	//	{
	//		WORD	Type
	//	}
	//------------------------------------------------------------
	PACKET_SC_LOGIN_RES_ECHO,

	//------------------------------------------------------
	// Game Server
	//------------------------------------------------------
	PACKET_GAME_SERVER = 1000,

	//------------------------------------------------------------
	//	{
	//		WORD	Type
	//		INT64	AccountNo
	//	}
	//------------------------------------------------------------
	PACKET_CS_GAME_REQ_LOGIM = 1001,


	//------------------------------------------------------------
	//	{
	//		WORD	Type
	//		INT64	AccountNo // 캐릭터 ID 캐릭터 PASSWORD
	//	}
	//------------------------------------------------------------
	PACKET_SC_GAME_RES_LOGIN = 1002
};






//#endif