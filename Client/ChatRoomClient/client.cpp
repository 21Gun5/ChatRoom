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
#include "base64.h"

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
void Client::send(DataPackType type, const char* data, uint32_t size)
{


	//// base64加密
	//std::string tmp = (char*)&dp;
	//std::string dpEncode = base64_encode(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.length());
	//::send(m_hSocket, dpEncode.c_str(), sizeof(dp), 0);
	//MessageBox(NULL, dpEncode.c_str(), NULL,0);
	// 异或加密
	//std::string buff = (char*)&dp;
	//::send(m_hSocket, encode(buff), sizeof(dp), 0);

	// 不加密
	//::send(m_hSocket, (char*)&dp, sizeof(dp), 0);
	
	//if (size == -1)
	//{
	//	//size = strlen(data);
	//	size = sizeof(dataEncode);
	//}



	// base64加密
	std::string tmp = data;
	std::string dataEncode = base64_encode(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.length());

	size = sizeof(dataEncode);
	DataPack dp = { type,size };

	::send(m_hSocket, (char*)&dp, sizeof(dp), 0);
	::send(m_hSocket, dataEncode.c_str(), sizeof(dataEncode), 0);
	// 不加密
	//::send(m_hSocket, data, size, 0);
	// 异或加密
	//std::string buff2 = data;
	//::send(m_hSocket, encode(buff2), size, 0);

}
DataPackResult* Client::recv()
{
	DataPackResult head = { 0 };
	//// 收到后先解密
	//int recvSize = ::recv(m_hSocket,(char*)&head, sizeof(head) - 1, 0);
	//std::string base64Str = (char*)&head;
	//std::string headDecode = base64_decode(base64Str);
	//if (recvSize != sizeof(headDecode) - 1)
	//{
	//	return NULL;
	//}

	if (::recv(m_hSocket, (char*)&head, sizeof(head) - 1, 0) != sizeof(head) - 1)
	{
		return NULL;
	}

	DataPackResult* pBuff = (DataPackResult*)malloc(sizeof(DataPackResult) + head.size);
	memset(pBuff, 0, head.size + sizeof(DataPackResult));
	memcpy(pBuff, &head, sizeof(head));

	//// 收到后先解密
	//int recvSize = ::recv(m_hSocket, pBuff->data, sizeof(head) - 1, 0);
	//std::string base64Str; //中间变量
	//// pBuff->data
	//base64Str = pBuff->data;
	//std::string pBuffDataDecode = base64_decode(base64Str);
	//if (recvSize != pBuff->size)
	//{
	//	free(pBuff);
	//	return NULL;
	//}

	if (::recv(m_hSocket, pBuff->data, pBuff->size, 0) != pBuff->size) 
	{
		free(pBuff);
		return NULL;
	}
	return (DataPackResult*)pBuff;
}
//DataPackResult* Client::recv()
//{
//	DataPackResult head = { 0 };
//	
//	// 收到后先解密
//	DataPackResult * phead = &head;
//	int recvSize = ::recv(m_hSocket, (char*)phead, sizeof(head) - 1, 0);
//
//	std::string tmp = (char*)phead;
//	std::string pheadDecode = base64_decode(tmp);
//	head = (DataPackResult )*pheadDecode.c_str();
//	//head = *headDecode.c_str();
//	//phead = *ppheadDecode;
//	//memcpy(head, &headDecode, sizeof(headDecode));
//	if (recvSize != sizeof(head) - 1)
//	{
//		return NULL;
//	}
//	/*if (::recv(m_hsocket, (char*)&head, sizeof(head) - 1, 0) != sizeof(head) - 1)
//	{
//		return null;
//	}*/
//
//	DataPackResult* pBuff = (DataPackResult*)malloc(sizeof(DataPackResult) + head.size);
//	memset(pBuff, 0, head.size + sizeof(DataPackResult));
//	memcpy(pBuff, &head, sizeof(head));
//
//	// 收到后先解密
//	//char * ppBuffDate = &( pBuff->data);
//	char * buff=NULL;
//	int recvSize2 = ::recv(m_hSocket, buff, sizeof(head) - 1, 0);
//	std::string base64Str = buff;
//	std::string pBuffDataDecode = base64_decode(base64Str);
//	memcpy(pBuff->data, &buff, sizeof(buff));
//	if (recvSize2 != pBuff->size)
//	{
//		free(pBuff);
//		return NULL;
//	}
//	return (DataPackResult*)pBuff;
//}
void Client::freeResult(DataPackResult* p) {
	free(p);
}

// 功能函数
void Login(Client* client, const char* pAccount, const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);//账号、密码
	client->send(login, buffer, buffer.GetLength());
}
void Register(Client* client, const char* pAccount, const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);//账号、密码
	client->send(registe,buffer,buffer.GetLength());
}
void SendMultiMsg(Client* pClient, const char* pMsg)
{
	// 基础版，只发消息
	pClient->send(sendMultiMsg, pMsg);// 消息
}
void SendMsg(Client* pClient, const char* pMsg, CString fromWhere, CString toWhere)
{
	// 进阶版，发送消息、发送者、接收者
	CStringA buffer;
	buffer.Format("%s\n%s\n%s", pMsg, fromWhere, toWhere);//账号、密码
	pClient->send(sendMsg, buffer, buffer.GetLength());
}
void AddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n0", pFriendName);// 好友名字、0（表示添加好友）
	pClient->send(addFriend, buf, buf.GetLength());
}
void AccpetAddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n1", pFriendName);// 好友名字、1（表示同意添加）
	pClient->send(addFriend, buf, buf.GetLength());
}
void RefuseAddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n2", pFriendName);// 好友名字、2（表示拒绝添加）
	pClient->send(addFriend, buf, buf.GetLength());
}
void GetFriendList(Client* pClient)
{
	pClient->send(getFriendList, "");// 空
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
			//base64_decode(base64_decode(pResult->data).c_str()).c_str()
			sprintf_s(buff, "失败:%s\n", base64_decode(pResult->data).c_str());
			tips = buff;

			MessageBox(NULL, tips, "提示", 0);
			continue;
		}

		switch (pResult->type)
		{
		case login:
		{
			g_isLogin = true;
			MessageBox(NULL, "登录成功", "提示", MB_OK);
			// 登录成功打开聊天框，此时登录框隐藏，关闭聊天框时再显示
			AfxGetApp()->GetMainWnd()->ShowWindow(SW_HIDE);
			break;
		}
		case registe:
		{
			MessageBox(NULL, "注册成功", "提示", MB_OK);
			break;
		}
		}
	}
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
			sprintf_s(buff, "失败:%s\n", base64_decode(pResult->data).c_str());
			tips = buff;

			MessageBox(NULL, tips, "提示", 0);
			continue;
		}
		switch (pResult->type)
		{
		case sendMultiMsg:// 广播消息
		{
			//来自服务端通知
			if (pResult->status == -1)
			{
				CString tips;
				sprintf_s(g_recvMessage, "%s\n", base64_decode(pResult->data).c_str());
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
				pEditRecord->SetWindowText(oldRecord + "\r\n" 
					+ strTime + "[Recv From All]" +"\r\n" 
					+ ">  "+ g_recvMessage);
			}
			else
			{
				//服务端的回复
				printf("\t消息发送成功\n");
			}
			break;
		}		
		case sendMsg:// 单播消息
		{
			//来自服务端通知
			if (pResult->status == -1)
			{
				// 提取消息
				sprintf_s(g_recvMessage, "%s\n", base64_decode(pResult->data).c_str());
				// 新+旧+时间，再显示在聊天记录框
				CEdit * pEditRecord = g_pEditChatRecord;// 从全局变量获取控件句柄
				CString strTime = CTime::GetCurrentTime().Format("%Y-%m-%d %X   ");
				CString oldRecord;
				pEditRecord->GetWindowText(oldRecord);
				pEditRecord->SetWindowText(oldRecord + "\r\n"
					+ strTime + "[Recv]" + "\r\n"
					+ ">  " + g_recvMessage);
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
				tips.Format("用户[%s] 要添加你为好友,是否同意?",base64_decode(pResult->data).c_str());

				// 弹出`是否`对话框窗来获取用户的选择
				if (IDYES == MessageBoxA(0, tips, "提示", MB_YESNO))
				{
					// 选择了是, 就将同意好友添加请求发送给服务端
					AccpetAddFriend(g_pClient, base64_decode(pResult->data).c_str());
				}
				else 
				{
					// 选择了否, 就将同意好友添加请求发送给服务端
					RefuseAddFriend(g_pClient, base64_decode(pResult->data).c_str());
				}
			}
			else 
			{
				printf("添加好友的结果: %s\n", base64_decode(pResult->data).c_str());
			}
			break;
		}
		case getFriendList:// 获取好友列表
		{
			//HWND hFriendList = GetDlgItem(g_hWndChat, IDC_EDIT_FINDFRIEND);
			CListCtrl * pFriendList = g_pListFriendList;// 从全局变量获取控件句柄
			// 先清空
			pFriendList->DeleteAllItems();
			// 切割好友列表
			char* context;
			char *p;
			char data[100];
			memcpy_s(data, 100, base64_decode(pResult->data).c_str(),100);
			p = strtok_s(data, "\n", &context);// 用2将1分隔得3
			int i = 1;
			// 将其填充至好友列表
			while (p != nullptr)
			{
				// int转LPCTSTR
				CString iStr;
				iStr.Format("%d", i);
				//LPCTSTR ii = str.AllocSysString();
				//// char * 转LPCTSTR
				//int num = MultiByteToWideChar(0, 0, p, -1, NULL, 0);
				//wchar_t *pp = new wchar_t[num];
				//MultiByteToWideChar(0, 0, p, -1, pp, num);
				// 插入行、设置内容
				pFriendList->InsertItem(0, iStr);
				pFriendList->SetItemText(0, 1, p);

				i++;
				p = strtok_s(nullptr, "\n", &context);
			}
			break;

			



		}

		}
	}
	return 0;
}