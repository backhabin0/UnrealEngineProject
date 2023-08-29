#include "IocpMain.h"
#include "protocol.h"



IocpMain::IocpMain() :iocp_handle(nullptr), gserver_socket(0), gclient_socket(0) {};

IocpMain::~IocpMain() { closesocket(gserver_socket); WSACleanup(); }

bool IocpMain::ServerInit() 
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	gserver_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(gserver_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(gserver_socket, SOMAXCONN);

	return true;
}

HANDLE IocpMain::CreateiocpHandle() { return CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0); }

bool IocpMain::RegisterdSocketOnHandle(SOCKET sock, ULONG_PTR key)
{
	HANDLE handle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(sock), iocp_handle, key, 0);
	return (handle == iocp_handle);
}

void ProcessPacket(int client_id, char* packet)
{
	switch (packet[1]) {
	case static_cast<int>(PACKET_TYPE::CS_LOGIN):
		CS_LOGIN_PACKET* packet = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		
		//추후 중복로그인 방지 코드 작성

		{
			std::lock_guard<std::mutex> lock{ clients[client_id].state_lock };
			clients[client_id].state = ST_INGAME;
		}

	}
}

void IocpMain::WorkerThread()
{
	DWORD num_bytes;
	ULONG_PTR key;
	WSAOVERLAPPED* over = nullptr;

	while (true) {
		BOOL ret = GetQueuedCompletionStatus(iocp_handle, &num_bytes, &key, &over, INFINITE);
		SocketInfo* ex_over = reinterpret_cast<SocketInfo*>(over);
		if (FALSE == ret) {
			if (ex_over->comp_type == OP_ACCEPT) std::cout << "Accept Error";
			else {
				std::cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		if ((0 == num_bytes) && ((ex_over->comp_type == OP_RECV) || (ex_over->comp_type == OP_SEND))) {
			disconnect(static_cast<int>(key));
			if (ex_over->comp_type == OP_SEND) delete ex_over;
			continue;
		}

		switch (ex_over->comp_type) {
		case OP_ACCEPT: {
			int client_id = GetClientId();

			if (-1 == client_id) {
				{
					std::lock_guard<std::mutex> lock(clients[client_id].state_lock);
					clients[client_id].state = ST_ALLOC;
				}

				clients[client_id].SessionId = client_id;
				clients[client_id].prev_remain_data = 0;

				if (!RegisterdSocketOnHandle(gclient_socket, client_id)) continue;
				clients[client_id].do_recv();
				gclient_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				std::cout << "Max user" << std::endl;
			}

			//socketinfo.wsabuf.len = BUF_SIZE;
			//socketinfo.comp_type = OP_ACCEPT;
			ZeroMemory(&socketinfo.over, sizeof(socketinfo.over));
			AcceptEx(gserver_socket, gclient_socket, socketinfo.send_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, 0, &socketinfo.over);
		}
					  break;
		case OP_RECV: {
			int remain_data = num_bytes + clients[key].prev_remain_data;
			char* p = ex_over->send_buf;

			while (remain_data > 0) {
				int packetsize = p[0];
				if (packetsize <= remain_data) {
					ProcessPacket(static_cast<int>(key), p);
					p = p + packetsize;
					remain_data = remain_data - packetsize;
				}
				else break;
			}
			clients[key].prev_remain_data = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->send_buf, p, remain_data);
			}
			clients[key].do_recv();
		}
					break;
		case OP_SEND: {
			delete ex_over;
		}
					break;
		}
	}
}

void IocpMain::ServerRun()
{
	gclient_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	socketinfo.comp_type = OP_ACCEPT;
	AcceptEx(gserver_socket, gclient_socket, socketinfo.send_buf, 0, addr_size + 16, addr_size + 16, 0, &socketinfo.over);

	std::vector<std::thread> worker_threads;
	int thread_num = std::thread::hardware_concurrency();

	for (int i = 0; i < thread_num; ++i) 
		worker_threads.emplace_back(std::thread(&IocpMain::WorkerThread, iocp_handle));
	for (auto& th : worker_threads)
		th.join();
}
