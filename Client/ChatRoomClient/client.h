#pragma once


// client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <conio.h>
#include <atlstr.h>
//#include "data.h"
#pragma comment(lib,"ws2_32.lib")


extern bool g_isLogin;
extern char g_recvMessage[200];

enum DataPackType {
	login = 1,//登录类型
	registe = 2,
	sendMultiMsg = 3,
};

#pragma pack(push,1)
struct DataPack
{
	DataPackType type;
	uint32_t	 size;
};

struct DataPackResult {
	uint32_t	type;
	int32_t		status;
	uint32_t	size;
	char		data[1];
};
#pragma pack(pop)


class Client {

	SOCKET m_hSocket;
public:
	static void initSocket()
	{
		WSADATA wd = { 0 };
		WSAStartup(MAKEWORD(2, 2), &wd);
	}

	Client(const char* serverIp, short port);
	void send(DataPackType type, const char* data, int size = -1);
	DataPackResult* recv();
	void freeResult(DataPackResult* p);
};


void Login(Client* client,const char* pAccount,const char* password);

void Register(Client* client,const char* pAccount,const char* password);

void SendMsg(Client* pClient, const char* pMsg);

DWORD CALLBACK recvProc(LPVOID arg);

