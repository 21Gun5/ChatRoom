//// client.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
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

// ȫ�ֱ���
bool g_isLogin = false;// �Ƿ��¼
char g_recvMessage[200] = { 0 };// ���յ�����Ϣ
HWND g_hWndChat = NULL;
CEdit * g_pEditChatRecord = NULL;
CListCtrl * g_pListFriendList= NULL;

// ��ĳ�Ա����
Client::Client(const char* serverIp, short port)
{
	Client::initSocket();
	// �����׽���
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// ���÷���˵ĵ�ַ�Ͷ˿�
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, serverIp, &addr.sin_addr);
	// ���ӵ������
	connect(m_hSocket, (sockaddr*)&addr, sizeof(addr));
}
void Client::send(DataPackType type, const char* data, uint32_t size)
{


	//// base64����
	//std::string tmp = (char*)&dp;
	//std::string dpEncode = base64_encode(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.length());
	//::send(m_hSocket, dpEncode.c_str(), sizeof(dp), 0);
	//MessageBox(NULL, dpEncode.c_str(), NULL,0);
	// ������
	//std::string buff = (char*)&dp;
	//::send(m_hSocket, encode(buff), sizeof(dp), 0);

	// ������
	//::send(m_hSocket, (char*)&dp, sizeof(dp), 0);
	
	//if (size == -1)
	//{
	//	//size = strlen(data);
	//	size = sizeof(dataEncode);
	//}



	// base64����
	std::string tmp = data;
	std::string dataEncode = base64_encode(reinterpret_cast<const unsigned char*>(tmp.c_str()), tmp.length());

	size = sizeof(dataEncode);
	DataPack dp = { type,size };

	::send(m_hSocket, (char*)&dp, sizeof(dp), 0);
	::send(m_hSocket, dataEncode.c_str(), sizeof(dataEncode), 0);
	// ������
	//::send(m_hSocket, data, size, 0);
	// ������
	//std::string buff2 = data;
	//::send(m_hSocket, encode(buff2), size, 0);

}
DataPackResult* Client::recv()
{
	DataPackResult head = { 0 };
	//// �յ����Ƚ���
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

	//// �յ����Ƚ���
	//int recvSize = ::recv(m_hSocket, pBuff->data, sizeof(head) - 1, 0);
	//std::string base64Str; //�м����
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
//	// �յ����Ƚ���
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
//	// �յ����Ƚ���
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

// ���ܺ���
void Login(Client* client, const char* pAccount, const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);//�˺š�����
	client->send(login, buffer, buffer.GetLength());
}
void Register(Client* client, const char* pAccount, const char* password)
{
	CStringA buffer;
	buffer.Format("%s\n%s", pAccount, password);//�˺š�����
	client->send(registe,buffer,buffer.GetLength());
}
void SendMultiMsg(Client* pClient, const char* pMsg)
{
	// �����棬ֻ����Ϣ
	pClient->send(sendMultiMsg, pMsg);// ��Ϣ
}
void SendMsg(Client* pClient, const char* pMsg, CString fromWhere, CString toWhere)
{
	// ���װ棬������Ϣ�������ߡ�������
	CStringA buffer;
	buffer.Format("%s\n%s\n%s", pMsg, fromWhere, toWhere);//�˺š�����
	pClient->send(sendMsg, buffer, buffer.GetLength());
}
void AddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n0", pFriendName);// �������֡�0����ʾ��Ӻ��ѣ�
	pClient->send(addFriend, buf, buf.GetLength());
}
void AccpetAddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n1", pFriendName);// �������֡�1����ʾͬ����ӣ�
	pClient->send(addFriend, buf, buf.GetLength());
}
void RefuseAddFriend(Client* pClient, const char* pFriendName)
{
	CStringA buf;
	buf.Format("%s\n2", pFriendName);// �������֡�2����ʾ�ܾ���ӣ�
	pClient->send(addFriend, buf, buf.GetLength());
}
void GetFriendList(Client* pClient)
{
	pClient->send(getFriendList, "");// ��
}

// �̴߳�������������Ϣ
DWORD CALLBACK recvLoginProc(LPVOID arg)
{
	Client* pClient = (Client*)arg;
	DataPackResult* pResult = NULL;

	// ��¼֮ǰ��ѭ��
	while (!g_isLogin)
	{

		pResult = pClient->recv();

		// û�յ��������
		if (pResult == NULL)
		{
			continue;
		}

		// ����ʧ���򵯴�����
		if (pResult->status > 0)
		{
			CString tips;
			char buff[100];
			//base64_decode(base64_decode(pResult->data).c_str()).c_str()
			sprintf_s(buff, "ʧ��:%s\n", base64_decode(pResult->data).c_str());
			tips = buff;

			MessageBox(NULL, tips, "��ʾ", 0);
			continue;
		}

		switch (pResult->type)
		{
		case login:
		{
			g_isLogin = true;
			MessageBox(NULL, "��¼�ɹ�", "��ʾ", MB_OK);
			// ��¼�ɹ�������򣬴�ʱ��¼�����أ��ر������ʱ����ʾ
			AfxGetApp()->GetMainWnd()->ShowWindow(SW_HIDE);
			break;
		}
		case registe:
		{
			MessageBox(NULL, "ע��ɹ�", "��ʾ", MB_OK);
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

	// ��¼���ѭ��
	while (g_isLogin)
	{
		pResult = g_pClient->recv();
		// û�յ��������
		if (pResult == NULL)
		{
			continue;
		}
		// ����ʧ���򵯴�����
		if (pResult->status > 0)
		{
			CString tips;
			char buff[100];
			sprintf_s(buff, "ʧ��:%s\n", base64_decode(pResult->data).c_str());
			tips = buff;

			MessageBox(NULL, tips, "��ʾ", 0);
			continue;
		}
		switch (pResult->type)
		{
		case sendMultiMsg:// �㲥��Ϣ
		{
			//���Է����֪ͨ
			if (pResult->status == -1)
			{
				CString tips;
				sprintf_s(g_recvMessage, "%s\n", base64_decode(pResult->data).c_str());
				tips = g_recvMessage;
				//MessageBox(NULL, tips, L"��ʾ", 0);

				//���յ�����Ϣ��ʾ�������¼��
				//CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat, IDC_EDIT_RECORD);//�������ؼ��������ȫ��
				CEdit * pEditRecord = g_pEditChatRecord;// ��ȫ�ֱ�����ȡ�ؼ����
				//��ȡϵͳʱ��
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
				//����˵Ļظ�
				printf("\t��Ϣ���ͳɹ�\n");
			}
			break;
		}		
		case sendMsg:// ������Ϣ
		{
			//���Է����֪ͨ
			if (pResult->status == -1)
			{
				// ��ȡ��Ϣ
				sprintf_s(g_recvMessage, "%s\n", base64_decode(pResult->data).c_str());
				// ��+��+ʱ�䣬����ʾ�������¼��
				CEdit * pEditRecord = g_pEditChatRecord;// ��ȫ�ֱ�����ȡ�ؼ����
				CString strTime = CTime::GetCurrentTime().Format("%Y-%m-%d %X   ");
				CString oldRecord;
				pEditRecord->GetWindowText(oldRecord);
				pEditRecord->SetWindowText(oldRecord + "\r\n"
					+ strTime + "[Recv]" + "\r\n"
					+ ">  " + g_recvMessage);
			}
			else
			{
				//����˵Ļظ�
				printf("\t��Ϣ���ͳɹ�\n");
			}
			break;
		}
		case addFriend:// ��Ӻ���
		{
			// ���״̬�����-1, ��ʾ���Ƿ����������ͻ��˷����֪ͨ.
			// ��ʾ�������ͻ��˷�������Ӻ��ѵ�����.
			if (pResult->status == -1) 
			{ // ���Է����֪ͨ: ��������Լ�Ϊ����
				CStringA tips;
				tips.Format("�û�[%s] Ҫ�����Ϊ����,�Ƿ�ͬ��?",base64_decode(pResult->data).c_str());

				// ����`�Ƿ�`�Ի�������ȡ�û���ѡ��
				if (IDYES == MessageBoxA(0, tips, "��ʾ", MB_YESNO))
				{
					// ѡ������, �ͽ�ͬ�������������͸������
					AccpetAddFriend(g_pClient, base64_decode(pResult->data).c_str());
				}
				else 
				{
					// ѡ���˷�, �ͽ�ͬ�������������͸������
					RefuseAddFriend(g_pClient, base64_decode(pResult->data).c_str());
				}
			}
			else 
			{
				printf("��Ӻ��ѵĽ��: %s\n", base64_decode(pResult->data).c_str());
			}
			break;
		}
		case getFriendList:// ��ȡ�����б�
		{
			//HWND hFriendList = GetDlgItem(g_hWndChat, IDC_EDIT_FINDFRIEND);
			CListCtrl * pFriendList = g_pListFriendList;// ��ȫ�ֱ�����ȡ�ؼ����
			// �����
			pFriendList->DeleteAllItems();
			// �и�����б�
			char* context;
			char *p;
			char data[100];
			memcpy_s(data, 100, base64_decode(pResult->data).c_str(),100);
			p = strtok_s(data, "\n", &context);// ��2��1�ָ���3
			int i = 1;
			// ��������������б�
			while (p != nullptr)
			{
				// intתLPCTSTR
				CString iStr;
				iStr.Format("%d", i);
				//LPCTSTR ii = str.AllocSysString();
				//// char * תLPCTSTR
				//int num = MultiByteToWideChar(0, 0, p, -1, NULL, 0);
				//wchar_t *pp = new wchar_t[num];
				//MultiByteToWideChar(0, 0, p, -1, pp, num);
				// �����С���������
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