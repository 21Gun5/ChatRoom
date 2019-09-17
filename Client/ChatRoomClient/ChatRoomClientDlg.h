
// ChatRoomClientDlg.h: 头文件
//

#pragma once
#include "client.h"

// 全局化client对象
extern Client * g_pClient ;
extern CString g_CurAccount;


// CChatRoomClientDlg 对话框
class CChatRoomClientDlg : public CDialogEx
{
// 构造
public:
	CChatRoomClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CHATROOMCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_userName;
	CEdit m_passWord;
	afx_msg void OnClickedButtonRegister();
	afx_msg void OnClickedButtonLogin();

//	static DWORD CALLBACK recvProc(LPVOID arg)
//
//	{
//		Client* pClient = (Client*)arg;
//		DataPackResult* pResult = NULL;
//
//		while (1)
//		{
//			pResult = pClient->recv();
//			if (pResult == NULL) {
//				continue;
//			}
//
//			if (pResult->status > 0) {
//				printf("[%d] 的处理失败:%s\n", pResult->type, pResult->data);
//
//				//char buff[50];
//				// (user_notify, "[%d] 的处理失败:%s\n", pResult->type, pResult->data);
//				//MessageBox(CChatRoomClientDlg,buff);
//
//				continue;
//			}
//
//			switch (pResult->type)
//			{
//			case login:
//				printf("登陆成功\n");
//				g_isLogin = true;
//				break;
//			case registe:
//				printf("注册成功\n");
//				user_notify = "注册成功\n";
//				MessageBox(user_notify);
//				//user_notify = "注册成功\n";
//				//sprintf_s(user_notify, "注册成功\n");
//				break;
//			case sendMultiMsg:
//			{
//				if (pResult->status == -1) { // 来自服务端通知
//					printf("有新的消息: %s\n", pResult->data);
//				}
//				else { // 服务端的回复
//					printf("\t消息发送成功\n");
//				}
//			}
//			}
//
//
//			printf("> ");
//		}
//		return 0;
//	}

};

