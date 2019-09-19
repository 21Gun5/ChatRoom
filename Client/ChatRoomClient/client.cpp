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
CListCtrl * g_pListRoomList = NULL;

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
	// base64加密
	std::string tmp = data;
	std::string dataEncode = base64_encode(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.length());

	size = sizeof(dataEncode);
	DataPack dp = { type,size };

	::send(m_hSocket, (char*)&dp, sizeof(dp), 0);
	::send(m_hSocket, dataEncode.c_str(), sizeof(dataEncode), 0);

}
DataPackResult* Client::recv()
{
	//DataPackResult head = { 0 };
	//if (::recv(m_hSocket, (char*)&head, sizeof(head) - 1, 0) != sizeof(head) - 1)
	//{
	//	return NULL;
	//}

	//DataPackResult* pBuff = (DataPackResult*)malloc(sizeof(DataPackResult) + head.size);
	//memset(pBuff, 0, head.size + sizeof(DataPackResult));
	//memcpy(pBuff, &head, sizeof(head));

	//if (::recv(m_hSocket, pBuff->data, pBuff->size, 0) != pBuff->size) 
	//{
	//	free(pBuff);
	//	return NULL;
	//}
	//return (DataPackResult*)pBuff;

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
	// 		for (int i = 0; i < pBuff->size; ++i) {
	// 			pBuff->data[i] ^= 0x15;
	// 		}
	return (DataPackResult*)pBuff;
}
void Client::freeResult(DataPackResult* p) {
	free(p);
}

// 注册登录
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
// 发消息
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
void SendRoomMsg(Client* pClient, const char* pMsg, CString fromWhere, CString toWhere)
{
	CStringA buff;
	// 注意: pMsg保存的额字符串中不能带有\n,否则就错误.
	//	    最好在发送前, 将\n转换成\\n再发送
	buff.Format("%s\n%s\n%s", pMsg, fromWhere, toWhere);
	pClient->send(sendMultiMsg, buff, buff.GetLength());
}
//void SendRoomMsg(Client* pClient, const char*roomName, const char* pMsg)
//{
//	CStringA buff;
//	// 注意: pMsg保存的额字符串中不能带有\n,否则就错误.
//	//	    最好在发送前, 将\n转换成\\n再发送
//	buff.Format("%s\n%s", roomName, pMsg);
//	pClient->send(sendMultiMsg, buff, buff.GetLength());
//}
// 好友相关
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
// 聊天室相关
void CreateRoom(Client* pClient, const char* roomName)
{
	pClient->send(createRoom, roomName);
}
void JoinRoom(Client* pClient, const char* roomName)
{
	pClient->send(joinRoom, roomName);
}
void GetRoomList(Client* pClient)
{
	pClient->send(getroomlist, "");
}
void GetRoomMembers(Client* pClient, const char* roomName)
{
	pClient->send(getroommember, roomName);
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


				//CString tips;
				//sprintf_s(g_recvMessage, "%s\n", base64_decode(pResult->data).c_str());
				//tips = g_recvMessage;
				////MessageBox(NULL, tips, L"提示", 0);
				////将收到的消息显示到聊天记录框
				////CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat, IDC_EDIT_RECORD);//出错，将控件句柄设置全局
				//CEdit * pEditRecord = g_pEditChatRecord;// 从全局变量获取控件句柄
				////获取系统时间
				//CString strTime;
				//CTime tm;
				//tm = CTime::GetCurrentTime();
				//strTime = tm.Format("%Y-%m-%d %X   ");
				//CString oldRecord;
				//pEditRecord->GetWindowText(oldRecord);
				//pEditRecord->SetWindowText(oldRecord + "\r\n" 
				//	+ strTime + "[Recv From All]" +"\r\n" 
				//	+ ">  "+ g_recvMessage);


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
				// 插入行、设置内容
				pFriendList->InsertItem(0, iStr);
				pFriendList->SetItemText(0, 1, p);

				i++;
				p = strtok_s(nullptr, "\n", &context);
			}
			break;
		}
		case createRoom:// 创建群
		{
			MessageBox(NULL, "创建成功", "提示", MB_OK);
			break;
		}
		case joinRoom:
		{
			MessageBox(NULL, "加入成功", "提示", MB_OK);
			break;
		}
		case getroomlist://获取群列表
		{

			//HWND hFriendList = GetDlgItem(g_hWndChat, IDC_EDIT_FINDFRIEND);
			CListCtrl * pRoomList = g_pListRoomList;// 从全局变量获取控件句柄
			// 先清空
			pRoomList->DeleteAllItems();
			// 切割好友列表
			char* context;
			char *p;
			char data[100];
			memcpy_s(data, 100, base64_decode(pResult->data).c_str(), 100);
			p = strtok_s(data, "\n", &context);// 用2将1分隔得3
			int i = 1;
			// 将其填充至好友列表
			while (p != nullptr)
			{
				// int转LPCTSTR
				CString iStr;
				iStr.Format("%d", i);
				// 插入行、设置内容
				pRoomList->InsertItem(0, iStr);
				pRoomList->SetItemText(0, 1, p);

				i++;
				p = strtok_s(nullptr, "\n", &context);
			}
			break;

			//// 切割群列表
			//char* context, *p;
			//p = strtok_s(pResult->data, "\n", &context);
			//int i = 1;

			//// 打印
			//printf("获取到的群列表: \n");
			//while (p != nullptr)
			//{
			//	printf("%d - %s\n", i++, p);
			//	p = strtok_s(nullptr, "\n", &context);
			//}
			//break;
		}
		case getroommember:// 获取群成员
		{
			// 切割群列表
			char* context, *p;
			p = strtok_s(pResult->data, "\n", &context);
			int i = 1;

			// 打印
			printf("获取到的群成员列表: \n");
			while (p != nullptr)
			{
				printf("%d - %s\n", i++, p);
				p = strtok_s(nullptr, "\n", &context);
			}
			break;
		}
		}
	}
	return 0;
}