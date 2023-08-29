#include "pch.h"
#include "FrameWork.h"

void SessionInfo::do_recv()
{
	DWORD recv_flag = 0;
	memset(&socketinfo.over, 0, sizeof(recv_socketinfo.over));
	recv_socketinfo.wsabuf.len = BUF_SIZE - prev_remain_data;
	recv_socketinfo.wsabuf.buf = recv_socketinfo.send_buf + prev_remain_data;
	WSARecv(socket, &recv_socketinfo.wsabuf, 1, 0, &recv_flag,
		&recv_socketinfo.over, 0);
}

void SessionInfo::do_send(void* packet)
{
	SocketInfo* send_data = new SocketInfo{ reinterpret_cast<char*>(packet) };
	WSASend(socket, &send_data->wsabuf, 1, 0, 0, &send_data->over, 0);
}

void SessionInfo::send_login_info_packet()
{
	SC_LOGIN_INFO_PACKET packet;
	packet.id = SessionId;
	packet.size = sizeof SC_LOGIN_INFO_PACKET;
	packet.type = SC_LOGIN_INFO;
	//packet.x = x;
	//packet.y = y;
	//packet.max_hp = max_hp;
	//packet.hp = hp;
	//packet.level = level;
	//packet.exp = exp;

	do_send(&packet);
}

void SessionInfo::send_add_player_packet(int c_id)
{
	SC_ADD_OBJECT_PACKET packet;
	packet.id = c_id;
	//strcpy_s(packet.name, clients[c_id].name);
	packet.size = sizeof packet;
	packet.type = SC_ADD_OBJECT;
	//packet.x = clients[c_id].x;
	//packet.y = clients[c_id].y;
	vl.lock();
	view_list.insert(c_id);
	vl.unlock();
	do_send(&packet);
}

void SessionInfo::send_remove_player_packet(int c_id)
{
	vl.lock();
	if (view_list.count(c_id)) {
		view_list.erase(c_id);
	}
	else {
		vl.unlock();
		return;
	}
	vl.unlock();
	SC_REMOVE_OBJECT_PACKET packet;
	packet.id = c_id;
	packet.size = sizeof packet;
	packet.type = SC_REMOVE_OBJECT;
	do_send(&packet);
}

void SessionInfo::send_login_fail_packet()
{
	SC_LOGIN_FAIL_PACKET packet;
	packet.size = sizeof packet;
	packet.type = SC_LOGIN_FAIL;
	do_send(&packet);
}

void disconnect(int c_id)
{
	clients[c_id].vl.lock();
	unordered_set<int> viewlist = clients[c_id].view_list;
	clients[c_id].vl.unlock();
	for (auto& p_id : viewlist) {
		//if (is_npc(p_id)) continue;
		auto& player = clients[p_id];
		{
			lock_guard<mutex> ll(player.state_lock);
			if (ST_INGAME != player.state) continue;
		}
		if (player.SessionId == c_id) continue;
		player.send_remove_player_packet(c_id);
	}
	closesocket(clients[c_id].socket);

	lock_guard<mutex> ll(clients[c_id].state_lock);
	clients[c_id].state = ST_FREE;
}

bool is_pc(int obj_id)
{
	return obj_id < MAX_USER;
}

bool is_npc(int object_id)
{
	return !is_pc(object_id);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i].state_lock };
		if (clients[i].state == ST_FREE)
			return i;
	}
	return -1;
}

bool can_attack(int from, int to)
{
	//if (abs(clients[from].x - clients[to].x) > 1) return false;
	//return abs(clients[from].y - clients[to].y) <= 1;
}