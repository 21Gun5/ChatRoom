#pragma once


// client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <WinSock2.h>
#include <Ws2tcpip.h>
#include <windows.h>
#include <conio.h>
#include <atlstr.h>
#include <string>
//#include "data.h"
#pragma comment(lib,"ws2_32.lib")

// 全局变量
extern bool g_isLogin;
extern char g_recvMessage[200];
extern HWND  g_hWndChat;
extern CEdit * g_pEditChatRecord;
extern CListCtrl * g_pListFriendList;
// 数据包类型
enum DataPackType {
	login = 1,//登录类型
	registe = 2,//注册
	sendMultiMsg = 3,// 广播
	sendMsg = 4,//单播
	addFriend = 5,// 加好友
	getFriendList = 6//获取好友列表
};
// 网络数据的结构
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
// 客户端socket
class Client {

	SOCKET m_hSocket;
public:
	static void initSocket()
	{
		WSADATA wd = { 0 };
		WSAStartup(MAKEWORD(2, 2), &wd);
	}

	Client(const char* serverIp, short port);
	void send(DataPackType type, const char* data, uint32_t size = -1);
	DataPackResult* recv();
	void freeResult(DataPackResult* p);
};
// 功能函数
void Login(Client* client,const char* pAccount,const char* password);
void Register(Client* client,const char* pAccount,const char* password);
void SendMultiMsg(Client* pClient, const char* pMsg);
void SendMsg(Client* pClient, const char* pMsg, CString fromWhere, CString toWhere);
void AddFriend(Client* pClient, const char* pFriendName);
void AccpetAddFriend(Client* pClient, const char* pFriendName);
void RefuseAddFriend(Client* pClient, const char* pFriendName);
void GetFriendList(Client* pClient);
// 回调函数（接收消息）
DWORD CALLBACK recvLoginProc(LPVOID arg);
DWORD CALLBACK recvMessageProc(LPVOID arg);

