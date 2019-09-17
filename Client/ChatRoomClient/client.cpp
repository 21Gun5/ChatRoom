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

// ���ܺ���
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
			sprintf_s(buff, "ʧ��:%s\n", pResult->data);
			tips = buff;

			MessageBox(NULL, tips, L"��ʾ", 0);
			continue;
		}

		switch (pResult->type)
		{
		case login:
		{
			g_isLogin = true;
			MessageBox(NULL, L"��¼�ɹ�", L"��ʾ", MB_OK);
			// ��¼�ɹ�������򣬴�ʱ��¼�����أ��ر������ʱ����ʾ
			AfxGetApp()->GetMainWnd()->ShowWindow(SW_HIDE);
			break;
		}
		case registe:
		{
			MessageBox(NULL, L"ע��ɹ�", L"��ʾ", MB_OK);
			break;
		}

		//case sendMultiMsg:
		//{
		//	if (pResult->status == -1) 
		//	{ 
		//		// ���Է����֪ͨ
		//		CString tips;
		//		sprintf_s(g_recvMessage, "����Ϣ: %s\n", pResult->data);
		//		tips = g_recvMessage;
		//		MessageBox(NULL, tips, L"��ʾ", 0);
		//		//// ���յ�����Ϣ��ʾ�������¼��
		//		//CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat,IDC_EDIT_RECODE);//�������ǿ������ת��
		//		//CString strTime; //��ȡϵͳʱ�� ����
		//		//CTime tm;
		//		//tm = CTime::GetCurrentTime();
		//		//strTime = tm.Format("%Y-%m-%d %X   ");
		//		//CString oldRecord;
		//		//pEditRecord->GetWindowText(oldRecord);
		//		//pEditRecord->SetWindowText(oldRecord + "\r\n" + strTime + "Recv:   " + g_recvMessage);
		//		
		//		//printf("���µ���Ϣ: %s\n", pResult->data);
		//	}
		//	else 
		//	{
		//		// ����˵Ļظ�
		//		printf("\t��Ϣ���ͳɹ�\n");
		//	}
		//}

		}
	}

	//// ��¼֮���ѭ��
	//while (g_isLogin)
	//{
	//	pResult = pClient->recv();
	//	// û�յ��������
	//	if (pResult == NULL)
	//	{
	//		continue;
	//	}
	//	// ����ʧ���򵯴�����
	//	if (pResult->status > 0)
	//	{
	//		CString tips;
	//		char buff[100];
	//		sprintf_s(buff, "ʧ��:%s\n", pResult->data);
	//		tips = buff;
	//		MessageBox(NULL, tips, L"��ʾ", 0);
	//		continue;
	//	}
	//	if (pResult->type == sendMultiMsg)
	//	{
	//		if (pResult->status == -1)
	//		{
	//			// ���Է����֪ͨ
	//			printf("���µ���Ϣ: %s\n", pResult->data);
	//		}
	//		else
	//		{
	//			// ����˵Ļظ�
	//			printf("\t��Ϣ���ͳɹ�\n");
	//		}
	//	}
	//}


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
			sprintf_s(buff, "ʧ��:%s\n", pResult->data);
			tips = buff;

			MessageBox(NULL, tips, L"��ʾ", 0);
			continue;
		}
		switch (pResult->type)
		{
		case sendMultiMsg:// ������Ϣ
		{
			//���Է����֪ͨ
			if (pResult->status == -1)
			{
				CString tips;
				sprintf_s(g_recvMessage, "%s\n", pResult->data);
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
				pEditRecord->SetWindowText(oldRecord + "\r\n" + strTime + "Recv:   " + g_recvMessage);
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
				tips.Format("�û�[%s] Ҫ�����Ϊ����,�Ƿ�ͬ��?",pResult->data);

				// ����`�Ƿ�`�Ի�������ȡ�û���ѡ��
				if (IDYES == MessageBoxA(0, tips, "��ʾ", MB_YESNO))
				{
					// ѡ������, �ͽ�ͬ�������������͸������
					AccpetAddFriend(g_pClient, pResult->data);
				}
				else 
				{
					// ѡ���˷�, �ͽ�ͬ�������������͸������
					RefuseAddFriend(g_pClient, pResult->data);
				}
			}
			else 
			{
				printf("��Ӻ��ѵĽ��: %s\n", pResult->data);
			}
			break;
		}
		case getFriendList:// ��ȡ�����б�
		{
			//HWND hFriendList = GetDlgItem(g_hWndChat, IDC_EDIT_FINDFRIEND);
			CListCtrl * pFriendList = g_pListFriendList;// ��ȫ�ֱ�����ȡ�ؼ����

			// �и�����б�
			char* context;
			char *p;
			p = strtok_s(pResult->data, "\n", &context);// ��2��1�ָ���3
			int i = 1;

			// ��������������б�
			while (p != nullptr)
			{
				// intתLPCTSTR
				CString str;
				str.Format(_T("%d"), i);
				LPCTSTR ii = str.AllocSysString();
				// char * תLPCTSTR
				int num = MultiByteToWideChar(0, 0, p, -1, NULL, 0);
				wchar_t *pp = new wchar_t[num];
				MultiByteToWideChar(0, 0, p, -1, pp, num);
				// �����С���������
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
		//	 ���Է����֪ͨ
		//	if (pResult->status == -1)
		//	{
		//		CString tips;
		//		sprintf_s(g_recvMessage, "%s\n", pResult->data);
		//		tips = g_recvMessage;
		//		MessageBox(NULL, tips, L"��ʾ", 0);
		//		 ���յ�����Ϣ��ʾ�������¼��
		//		CEdit * pEditRecord = (CEdit*)GetDlgItem(g_hpWndChat,IDC_EDIT_RECORD);//�������ؼ��������ȫ��
		//		CEdit * pEditRecord = g_hpEditChatRecord;// ��ȫ�ֱ�����ȡ�ؼ����
		//		��ȡϵͳʱ�� 
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
		//		 ����˵Ļظ�
		//		printf("\t��Ϣ���ͳɹ�\n");
		//	}
		//}
	}
	return 0;
}