#pragma once
#include "pch.h"
#include "FrameWork.h"

class IocpMain
{
	HANDLE iocp_handle;
	SOCKET gserver_socket, gclient_socket;
	SocketInfo socketinfo;

public:
	bool ServerInit();
	void ServerRun();

public:
	IocpMain();
	~IocpMain();

protected:
	HANDLE CreateiocpHandle(); 
	bool RegisterdSocketOnHandle(SOCKET sock, ULONG_PTR key);
	void ProcessPacket(int client_id, char* packet);
	void WorkerThread();
	void TimerThread();
	int GetClientId();
};

