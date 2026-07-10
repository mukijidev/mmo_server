#pragma once

#include "Type.h"

#define MONITOR_DEFAULT_PORT 10393

enum en_MONITOR_PACKET_TYPE
{
    // server -> monitor server
    PACKET_SS_MONITOR_LOGIN            = 30000, 
    PACKET_SS_MONITOR_DATA_UPDATE      = 30001, 
    PACKET_SS_MONITOR_SECTOR           = 30002, 

    // tool <-> monitor server
    PACKET_CS_MONITOR_TOOL_REQ_LOGIN   = 40000, // tool -> mon 
    PACKET_SC_MONITOR_TOOL_RES_LOGIN   = 40001, // mon -> tool 
    PACKET_SC_MONITOR_TOOL_DATA_UPDATE = 40002, // mon -> tool 
    PACKET_SC_MONITOR_TOOL_SECTOR      = 40003, // mon -> tool 
};

// Sector heatmap grids
enum en_MONITOR_SECTOR_GRID_ID
{
    SECTOR_GRID_LOBBY    = 0,
    SECTOR_GRID_GUARDIAN = 1,
    SECTOR_GRID_SPIDER   = 2,
};

// Server identifiers
enum en_MONITOR_SERVER_NO
{
    SERVER_NO_LOGIN = 1,
    SERVER_NO_GAME  = 2,
    SERVER_NO_CHAT  = 3,
};


enum en_MONITOR_DATA_TYPE
{
    // hardware 
    dfMONITOR_DATA_TYPE_SERVER_CPU            = 1,  // server CPU            
    dfMONITOR_DATA_TYPE_SERVER_AVAILABLE_MEM  = 2,  // server available mem  
    dfMONITOR_DATA_TYPE_SERVER_NONPAGED_MEM   = 3,  // Nonpaged Mem
    dfMONITOR_DATA_TYPE_SERVER_NETWORK_RECV   = 4,  // Network Recv
    dfMONITOR_DATA_TYPE_SERVER_NETWORK_SEND   = 5,  // Network Send

    // game server
    dfMONITOR_DATA_TYPE_GAME_SERVER_RUN       = 10, // game server ON/OFF
    dfMONITOR_DATA_TYPE_GAME_SERVER_CPU       = 11, // game server CPU
    dfMONITOR_DATA_TYPE_GAME_SERVER_MEM       = 12, // game server memory
    dfMONITOR_DATA_TYPE_GAME_PACKET_POOL      = 13, // PacketPool, usecount
    dfMONITOR_DATA_TYPE_GAME_ACCEPT_TPS       = 14, // 
    dfMONITOR_DATA_TYPE_GAME_AUTH_PLAYER      = 15, // Auth thread
    dfMONITOR_DATA_TYPE_GAME_GAME_PLAYER      = 16, // 
    dfMONITOR_DATA_TYPE_GAME_SESSION          = 17, // Session netlib
    dfMONITOR_DATA_TYPE_GAME_AUTH_THREAD_FPS  = 18, // Login FPS
    dfMONITOR_DATA_TYPE_GAME_GAME_THREAD_FPS  = 19, // Lobby FPS
    dfMONITOR_DATA_TYPE_GAME_DB_TPS           = 20, 
    dfMONITOR_DATA_TYPE_GAME_DB_QUEUE         = 21, /// DB job queue size
    dfMONITOR_DATA_TYPE_GAME_SEND_MSG_TPS     = 22, // send messages 
    dfMONITOR_DATA_TYPE_GAME_RECV_MSG_TPS     = 23, // recv messages
    dfMONITOR_DATA_TYPE_GAME_TOTAL_DISCONNECT = 24, // 
    dfMONITOR_DATA_TYPE_GAME_DISCONNECT_SENDQ_FULL = 25, // disconnect by send-queue full 

    // game server - JPS pathfinding
   
    dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_PLAYER_TPS       = 70, 
    dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_MONSTER_TPS      = 71, 
    dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_PLAYER_TPS    = 72, 
    dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_MONSTER_TPS   = 73, 
    dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_PLAYER_TPS      = 74, 
    dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_MONSTER_TPS     = 75, 
    dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_PLAYER_QUEUE     = 76, 
    dfMONITOR_DATA_TYPE_GAME_JPS_LOBBY_MONSTER_QUEUE    = 77, 
    dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_PLAYER_QUEUE  = 78, 
    dfMONITOR_DATA_TYPE_GAME_JPS_GUARDIAN_MONSTER_QUEUE = 79, 
    dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_PLAYER_QUEUE    = 80, 
    dfMONITOR_DATA_TYPE_GAME_JPS_SPIDER_MONSTER_QUEUE   = 81, 

    // game server - main game-loop tick time
    dfMONITOR_DATA_TYPE_GAME_FRAME_AVG  = 90, // ms   average tick time
    dfMONITOR_DATA_TYPE_GAME_FRAME_MAX  = 93, // ms   worst tick 

    // game server - dungeon monster counts
    dfMONITOR_DATA_TYPE_GAME_MONSTER_GUARDIAN_ALIVE = 100,
    dfMONITOR_DATA_TYPE_GAME_MONSTER_GUARDIAN_MAX   = 101, 
    dfMONITOR_DATA_TYPE_GAME_MONSTER_SPIDER_ALIVE   = 102, 
    dfMONITOR_DATA_TYPE_GAME_MONSTER_SPIDER_MAX     = 103,

    // chat server
    dfMONITOR_DATA_TYPE_CHAT_SERVER_RUN       = 30, 
    dfMONITOR_DATA_TYPE_CHAT_SERVER_CPU       = 31, 
    dfMONITOR_DATA_TYPE_CHAT_SERVER_MEM       = 32, 
    dfMONITOR_DATA_TYPE_CHAT_PACKET_POOL      = 33,
    dfMONITOR_DATA_TYPE_CHAT_SESSION          = 34, 
    dfMONITOR_DATA_TYPE_CHAT_PLAYER           = 35, 
    dfMONITOR_DATA_TYPE_CHAT_UPDATE_TPS       = 36, //  TPS job queue speed
    dfMONITOR_DATA_TYPE_CHAT_JOBQUEUE   = 37,  // jobqueue size

    // login server
    dfMONITOR_DATA_TYPE_LOGIN_SERVER_RUN      = 50, 
    dfMONITOR_DATA_TYPE_LOGIN_SERVER_CPU      = 51, 
    dfMONITOR_DATA_TYPE_LOGIN_SERVER_MEM      = 52,
    dfMONITOR_DATA_TYPE_LOGIN_PACKET_POOL     = 53, 
    dfMONITOR_DATA_TYPE_LOGIN_SESSION         = 54, 
    dfMONITOR_DATA_TYPE_LOGIN_AUTH_TPS        = 55, // auth TPS
};

#define MONITOR_LAN_PACKET_CODE  0x77 

#define MONITOR_SECTOR_GRID  15
