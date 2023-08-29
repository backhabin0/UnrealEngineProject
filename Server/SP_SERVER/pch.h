// pch.h: 미리 컴파일된 헤더 파일입니다.
// 아래 나열된 파일은 한 번만 컴파일되었으며, 향후 빌드에 대한 빌드 성능을 향상합니다.
// 코드 컴파일 및 여러 코드 검색 기능을 포함하여 IntelliSense 성능에도 영향을 미칩니다.
// 그러나 여기에 나열된 파일은 빌드 간 업데이트되는 경우 모두 다시 컴파일됩니다.
// 여기에 자주 업데이트할 파일을 추가하지 마세요. 그러면 성능이 저하됩니다.

#ifndef PCH_H
#define PCH_H

#pragma once


#pragma once

#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_set>
#include <set>
#include <cmath> 
#include <concurrent_priority_queue.h>
#include "protocol.h"
#include <sqlext.h>
#include <cwchar>
#include <unordered_map>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#define NAME_LEN 20 
#define MAXHP 300
//constexpr int  BUF_SIZE = 1024;

using namespace std;

//constexpr int VIEW_RANGE = 15;


class SocketInfo;
class SessionInfo;

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };
enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
enum EVENT_TYPE {  };

enum class PACKET_TYPE { CS_LOGIN };

struct TIMER_EVENT {
	int object_id;
	chrono::system_clock::time_point wakeup_time;
	EVENT_TYPE event;
	int target_id;
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};


extern concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
extern array<SessionInfo, MAX_USER + MAX_NPC> clients;
//extern int g_left_x;
//extern int g_top_y;
extern unordered_map<int, TIMER_EVENT> latest_timer_events;


// 여기에 미리 컴파일하려는 헤더 추가

#endif //PCH_H
