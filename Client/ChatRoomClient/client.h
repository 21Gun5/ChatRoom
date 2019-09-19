#pragma once


// client.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
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

// ȫ�ֱ���
extern bool g_isLogin;
extern char g_recvMessage[200];
extern HWND  g_hWndChat;
extern CEdit * g_pEditChatRecord;
extern CListCtrl * g_pListFriendList;
extern CListCtrl * g_pListRoomList;
// ���ݰ�����
enum DataPackType {
	login = 1,//��¼����
	registe = 2,//ע��
	sendMultiMsg = 3,// �㲥
	sendMsg = 4,//����
	addFriend = 5,// �Ӻ���
	getFriendList = 6,//��ȡ�����б�
	createRoom = 7,// ��������
	joinRoom = 8,//���뷿��
	getroomlist = 9,//��ȡ�����б�
	getroommember = 10//�õ�Ⱥ��Ա
};
// �������ݵĽṹ
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
// �ͻ���socket
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
// ע���¼
void Login(Client* client,const char* pAccount,const char* password);
void Register(Client* client,const char* pAccount,const char* password);
// ����Ϣ
void SendMultiMsg(Client* pClient, const char* pMsg);
void SendMsg(Client* pClient, const char* pMsg, CString fromWhere, CString toWhere);
//void SendRoomMsg(Client* pClient, const char*roomName, const char* pMsg);
void SendRoomMsg(Client* pClient, const char* pMsg, CString fromWhere, CString toWhere);
// �������
void AddFriend(Client* pClient, const char* pFriendName);
void AccpetAddFriend(Client* pClient, const char* pFriendName);
void RefuseAddFriend(Client* pClient, const char* pFriendName);
void GetFriendList(Client* pClient);
// ���������
void CreateRoom(Client* pClient, const char* roomName);
void JoinRoom(Client* pClient, const char* roomName);
void GetRoomList(Client* pClient);
void GetRoomMembers(Client* pClient, const char* roomName);

// �ص�������������Ϣ��
DWORD CALLBACK recvLoginProc(LPVOID arg);
DWORD CALLBACK recvMessageProc(LPVOID arg);

