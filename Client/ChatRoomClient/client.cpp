//// client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
////
//
#include "stdafx.h"
#include "client.h"
#include <atlstr.h>
#include <windows.h>
#include "CChatZone.h"

bool g_isLogin = false;// 是否登录
char g_recvMessage[200] = { 0 };// 接收到的消息

Client::Client(const char* serverIp, short port)
{
	Client::initSocket();
	// 创建套接字
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// 配置服务端的地址和端口
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, serverIp, &addr.sin_addr);
	// 连接到服务端
	connect(m_hSocket, (sockaddr*)&addr, sizeof(addr));
}

void Client::send(DataPackType type, const char* data, int size)
{
	if (size == -1) {
		size = strlen(data);
	}
	DataPack dp = { type,size };
	::send(m_hSocket, (char*)&dp, sizeof(dp), 0);
	::send(m_hSocket, data, size, 0);
}

DataPackResult* Client::recv()
{
	DataPackResult head = { 0 };
	if (::recv(m_hSocket, (char*)&head, sizeof(head) - 1, 0) != sizeof(head) - 1) {
		return NULL;
	}
	DataPackResult* pBuff = (DataPackResult*)malloc(sizeof(DataPackResult) + head.size);
	memset(pBuff, 0, head.size + sizeof(DataPackResult));
	memcpy(pBuff, &head, sizeof(head));
	if (::recv(m_hSocket, pBuff->data, pBuff->size, 0) != pBuff->size) {
		free(pBuff);
		return NULL;
	}
	return (DataPackResult*)pBuff;
}

void Client::freeResult(DataPackResult* p) {
	free(p);
}

void Login(Client* client,const char* pAccount,const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);
	client->send(login,buffer,buffer.GetLength());
}

void Register(Client* client,const char* pAccount,const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);
	client->send(registe,
		buffer,
		buffer.GetLength());
}

void SendMsg(Client* pClient, const char* pMsg)
{
	pClient->send(sendMultiMsg, pMsg);
}


DWORD CALLBACK recvProc(LPVOID arg)
{
	Client* pClient = (Client*)arg;
	DataPackResult* pResult = NULL;

	// 登录之前的循环
	while (1)
	{

		pResult = pClient->recv();

		// 没收到则继续收
		if (pResult == NULL) 
		{
			continue;
		}

		// 操作失败则弹窗提醒
		if (pResult->status > 0) 
		{
			CString tips;
			char buff[100];
			sprintf_s(buff, "失败:%s\n", pResult->data);
			tips = buff;

			MessageBox(NULL, tips, L"提示", 0);
			continue;
		}

		switch (pResult->type)
		{
		case login:
		{
			g_isLogin = true;
			MessageBox(NULL, L"登录成功", L"提示", MB_OK);

			// 登录成功打开聊天框，此时登录框隐藏，关闭聊天框时再显示
			AfxGetApp()->GetMainWnd()->ShowWindow(SW_HIDE);

			//CWnd* phWnd = AfxGetApp()->GetMainWnd(); //获取主窗口的句柄
			//ShowWindow(*phWnd,SW_HIDE);// 隐藏主窗口（当关闭会话框时再显示
			

			if (g_isLogin)
			{	
				CChatZone chatZoneDlg(NULL);// 创建模块对话框
				chatZoneDlg.DoModal();// 运行对话框
			}

			break;
		}
		case registe:
		{
			MessageBox(NULL, L"注册成功", L"提示", MB_OK);
			break;
		}
		case sendMultiMsg:
		{
			if (pResult->status == -1) 
			{ 
				// 来自服务端通知
				//CString tips;
				//char buff[100];
				sprintf_s(g_recvMessage, "新消息: %s\n", pResult->data);
				//tips = buff;

				//MessageBox(NULL, tips, L"提示", 0);

				//printf("有新的消息: %s\n", pResult->data);
			}
			else 
			{
				// 服务端的回复
				printf("\t消息发送成功\n");
			}
		}
		}

		//printf("> ");


	}
	
	//// 登录之后的循环
	//while (g_isLogin)
	//{
	//	pResult = pClient->recv();
	//	// 没收到则继续收
	//	if (pResult == NULL)
	//	{
	//		continue;
	//	}
	//	// 操作失败则弹窗提醒
	//	if (pResult->status > 0)
	//	{
	//		CString tips;
	//		char buff[100];
	//		sprintf_s(buff, "失败:%s\n", pResult->data);
	//		tips = buff;
	//		MessageBox(NULL, tips, L"提示", 0);
	//		continue;
	//	}
	//	if (pResult->type == sendMultiMsg)
	//	{
	//		if (pResult->status == -1)
	//		{
	//			// 来自服务端通知
	//			printf("有新的消息: %s\n", pResult->data);
	//		}
	//		else
	//		{
	//			// 服务端的回复
	//			printf("\t消息发送成功\n");
	//		}
	//	}
	//}
	

	return 0;
}



