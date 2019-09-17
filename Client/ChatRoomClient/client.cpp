//// client.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
////
//
#include "stdafx.h"
#include "client.h"
#include <atlstr.h>
#include <windows.h>
#include "CChatDlg.h"
#include "resource.h"
#include "ChatRoomClientDlg.h"

// 全局变量
bool g_isLogin = false;// 是否登录
char g_recvMessage[200] = { 0 };// 接收到的消息
HWND g_hWndChat = NULL;
CEdit * g_pEditChatRecord = NULL;
CListCtrl * g_pListFriendList= NULL;

// 类的成员函数
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

// 功能函数
void Login(Client* client, const char* pAccount, const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);
	client->send(login, buffer, buffer.GetLength());
}
void Register(Client* client, const char* pAccount, const char* password)
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
void AddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n0", pFriendName);
	pClient->send(addFriend, buf, buf.GetLength());
}
void AccpetAddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n1", pFriendName);
	pClient->send(addFriend, buf, buf.GetLength());
}
void RefuseAddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n2", pFriendName);
	pClient->send(addFriend, buf, buf.GetLength());
}
void GetFriendList(Client* pClient)
{
	pClient->send(getFriendList, "");
}

// 线程处理函数（接收消息
DWORD CALLBACK recvLoginProc(LPVOID arg)
{
	Client* pClient = (Client*)arg;
	DataPackResult* pResult = NULL;

	// 登录之前的循环
	while (!g_isLogin)
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
			break;
		}
		case registe:
		{
			MessageBox(NULL, L"注册成功", L"提示", MB_OK);
			break;
		}

		//case sendMultiMsg:
		//{
		//	if (pResult->status == -1) 
		//	{ 
		//		// 来自服务端通知
		//		CString tips;
		//		sprintf_s(g_recvMessage, "新消息: %s\n", pResult->data);
		//		tips = g_recvMessage;
		//		MessageBox(NULL, tips, L"提示", 0);
		//		//// 将收到的消息显示到聊天记录框
		//		//CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat,IDC_EDIT_RECODE);//必须进行强制类型转换
		//		//CString strTime; //获取系统时间 　　
		//		//CTime tm;
		//		//tm = CTime::GetCurrentTime();
		//		//strTime = tm.Format("%Y-%m-%d %X   ");
		//		//CString oldRecord;
		//		//pEditRecord->GetWindowText(oldRecord);
		//		//pEditRecord->SetWindowText(oldRecord + "\r\n" + strTime + "Recv:   " + g_recvMessage);
		//		
		//		//printf("有新的消息: %s\n", pResult->data);
		//	}
		//	else 
		//	{
		//		// 服务端的回复
		//		printf("\t消息发送成功\n");
		//	}
		//}

		}
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
DWORD CALLBACK recvMessageProc(LPVOID arg)
{

	//Client pClient = *(Client*)arg;
	DataPackResult* pResult = NULL;

	// 登录后的循环
	while (g_isLogin)
	{
		pResult = g_pClient->recv();
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
		case sendMultiMsg:// 发送消息
		{
			//来自服务端通知
			if (pResult->status == -1)
			{
				CString tips;
				sprintf_s(g_recvMessage, "%s\n", pResult->data);
				tips = g_recvMessage;
				//MessageBox(NULL, tips, L"提示", 0);

				//将收到的消息显示到聊天记录框
				//CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat, IDC_EDIT_RECORD);//出错，将控件句柄设置全局
				CEdit * pEditRecord = g_pEditChatRecord;// 从全局变量获取控件句柄
				//获取系统时间
				CString strTime;
				CTime tm;
				tm = CTime::GetCurrentTime();
				strTime = tm.Format("%Y-%m-%d %X   ");
				CString oldRecord;
				pEditRecord->GetWindowText(oldRecord);
				pEditRecord->SetWindowText(oldRecord + "\r\n" + strTime + "Recv:   " + g_recvMessage);
			}
			else
			{
				//服务端的回复
				printf("\t消息发送成功\n");
			}
			break;
		}		
		case addFriend:// 添加好友
		{
			// 如果状态码等于-1, 表示这是服务端主动向客户端发起的通知.
			// 表示有其它客户端发来了添加好友的请求.
			if (pResult->status == -1) 
			{ // 来自服务端通知: 有人添加自己为好友
				CStringA tips;
				tips.Format("用户[%s] 要添加你为好友,是否同意?",pResult->data);

				// 弹出`是否`对话框窗来获取用户的选择
				if (IDYES == MessageBoxA(0, tips, "提示", MB_YESNO))
				{
					// 选择了是, 就将同意好友添加请求发送给服务端
					AccpetAddFriend(g_pClient, pResult->data);
				}
				else 
				{
					// 选择了否, 就将同意好友添加请求发送给服务端
					RefuseAddFriend(g_pClient, pResult->data);
				}
			}
			else 
			{
				printf("添加好友的结果: %s\n", pResult->data);
			}
			break;
		}
		case getFriendList:// 获取好友列表
		{
			//HWND hFriendList = GetDlgItem(g_hWndChat, IDC_EDIT_FINDFRIEND);
			CListCtrl * pFriendList = g_pListFriendList;// 从全局变量获取控件句柄

			// 切割好友列表
			char* context;
			char *p;
			p = strtok_s(pResult->data, "\n", &context);// 用2将1分隔得3
			int i = 1;

			// 将其填充至好友列表
			while (p != nullptr)
			{
				// int转LPCTSTR
				CString str;
				str.Format(_T("%d"), i);
				LPCTSTR ii = str.AllocSysString();
				// char * 转LPCTSTR
				int num = MultiByteToWideChar(0, 0, p, -1, NULL, 0);
				wchar_t *pp = new wchar_t[num];
				MultiByteToWideChar(0, 0, p, -1, pp, num);
				// 插入行、设置内容
				pFriendList->InsertItem(0, ii);
				pFriendList->SetItemText(0, 1, pp);

				i++;
				p = strtok_s(nullptr, "\n", &context);
			}
			break;

			



		}

		}

		//if (pResult->type == sendMultiMsg)
		//{
		//	 来自服务端通知
		//	if (pResult->status == -1)
		//	{
		//		CString tips;
		//		sprintf_s(g_recvMessage, "%s\n", pResult->data);
		//		tips = g_recvMessage;
		//		MessageBox(NULL, tips, L"提示", 0);
		//		 将收到的消息显示到聊天记录框
		//		CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat,IDC_EDIT_RECORD);//出错，将控件句柄设置全局
		//		CEdit * pEditRecord = g_hpEditChatRecord;// 从全局变量获取控件句柄
		//		获取系统时间 
		//		CString strTime;
		//		CTime tm;
		//		tm = CTime::GetCurrentTime();
		//		strTime = tm.Format("%Y-%m-%d %X   ");
		//		CString oldRecord;
		//		pEditRecord->GetWindowText(oldRecord);
		//		pEditRecord->SetWindowText(oldRecord + "\r\n" + strTime + "Recv:   " + g_recvMessage);
		//	}
		//	else
		//	{
		//		 服务端的回复
		//		printf("\t消息发送成功\n");
		//	}
		//}
	}
	return 0;
}