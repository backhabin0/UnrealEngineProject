#pragma once
#include "protocol.h"
// 멀티바이트 집합 사용시 define
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// winsock2 사용을 위해 아래 코멘트 추가
#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>

using namespace std;



//enum COMMONCLASS_API
//{
//	LOGIN,
//	ENROLL_PLAYER,
//	SEND_PLAYER,
//	RECV_PLAYER,
//	LOGOUT_PLAYER,
//	HIT_PLAYER,
//	DAMAGED_PLAYER,
//	CHAT,
//	ENTER_NEW_PLAYER,
//	SIGNUP,
//	HIT_MONSTER,
//	SYNC_MONSTER,
//	SPAWN_MONSTER,
//	DESTROY_MONSTER
//};

class SocketInfo
{
public:
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	char send_buf[BUF_SIZE];
	COMP_TYPE comp_type;

	SocketInfo() {
		wsabuf.buf = send_buf;
		wsabuf.len = BUF_SIZE;
		ZeroMemory(&over, sizeof(over));
	}

	SocketInfo(char* packet) {
		wsabuf.buf = send_buf;
		wsabuf.len = packet[0];
		ZeroMemory(&over, sizeof(over));
		memcpy(send_buf, packet, packet[0]);
	}
};

extern SocketInfo socketinfo;

class SessionInfo
{
	SocketInfo recv_socketinfo;
public:
	SOCKET socket;
	int prev_remain_data;
	S_STATE state;
	unordered_set<int> view_list;
	mutex state_lock;	// STATE 전용 mutex
	mutex vl;	//viewlist 전용 mutex
	
public:
	// 세션 아이디
	int		SessionId;
	// 위치
	float	X;
	float	Y;
	float	Z;
	// 회전값
	float	Yaw;
	float	Pitch;
	float	Roll;
	// 속도
	float VX;
	float VY;
	float VZ;
	//// 속성
	//bool	IsAlive;
	//float	HealthValue;
	//bool	IsAttacking;

	//friend ostream& operator<<(ostream& stream, SessionInfo& info)
	//{
	//	stream << info.SessionId << endl;
	//	stream << info.X << endl;
	//	stream << info.Y << endl;
	//	stream << info.Z << endl;
	//	stream << info.VX << endl;
	//	stream << info.VY << endl;
	//	stream << info.VZ << endl;
	//	stream << info.Yaw << endl;
	//	stream << info.Pitch << endl;
	//	stream << info.Roll << endl;
	//	stream << info.IsAlive << endl;
	//	stream << info.HealthValue << endl;
	//	stream << info.IsAttacking << endl;

	//	return stream;
	//}

	friend istream& operator>>(istream& stream, SessionInfo& info)
	{
		stream >> info.SessionId;
		stream >> info.X;
		stream >> info.Y;
		stream >> info.Z;
		stream >> info.VX;
		stream >> info.VY;
		stream >> info.VZ;
		stream >> info.Yaw;
		stream >> info.Pitch;
		stream >> info.Roll;
	/*	stream >> info.IsAlive;
		stream >> info.HealthValue;
		stream >> info.IsAttacking;*/

		return stream;
	}
	SessionInfo() {};
	~SessionInfo() {};

	void do_recv();
	void do_send(void* packet);
	void send_login_info_packet();
	//void send_move_packet(int c_id);
	void send_add_player_packet(int c_id);
	//void send_chat_packet(int c_id, const char* mess);
	void send_remove_player_packet(int c_id);
	void send_login_fail_packet();
	//void send_state_change_packet(int c_id);
	//void send_player_die_packet(int c_id);
	//void send_pc_die_packet(int pc_id);
	
};
 

void disconnect(int c_id);
bool can_see(int from, int to);
int get_new_client_id();
bool can_attack(int from, int to); 