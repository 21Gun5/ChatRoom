//// client.cpp : ���ļ����� "main" ����������ִ�н��ڴ˴���ʼ��������
////
//
#include "stdafx.h"
#include "client.h"
#include <atlstr.h>
#include <windows.h>
#include "CChatZone.h"

bool g_isLogin = false;// �Ƿ��¼
char g_recvMessage[200] = { 0 };// ���յ�����Ϣ

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

	// ��¼֮ǰ��ѭ��
	while (1)
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

			//CWnd* phWnd = AfxGetApp()->GetMainWnd(); //��ȡ�����ڵľ��
			//ShowWindow(*phWnd,SW_HIDE);// ���������ڣ����رջỰ��ʱ����ʾ
			

			if (g_isLogin)
			{	
				CChatZone chatZoneDlg(NULL);// ����ģ��Ի���
				chatZoneDlg.DoModal();// ���жԻ���
			}

			break;
		}
		case registe:
		{
			MessageBox(NULL, L"ע��ɹ�", L"��ʾ", MB_OK);
			break;
		}
		case sendMultiMsg:
		{
			if (pResult->status == -1) 
			{ 
				// ���Է����֪ͨ
				//CString tips;
				//char buff[100];
				sprintf_s(g_recvMessage, "����Ϣ: %s\n", pResult->data);
				//tips = buff;

				//MessageBox(NULL, tips, L"��ʾ", 0);

				//printf("���µ���Ϣ: %s\n", pResult->data);
			}
			else 
			{
				// ����˵Ļظ�
				printf("\t��Ϣ���ͳɹ�\n");
			}
		}
		}

		//printf("> ");


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



