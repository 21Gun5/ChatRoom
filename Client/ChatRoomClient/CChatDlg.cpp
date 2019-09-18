// CChatZone.cpp: 实现文件
//

#include "stdafx.h"
#include "ChatRoomClient.h"
#include "CChatDlg.h"
#include "afxdialogex.h"
#include "client.h"
#include "ChatRoomClientDlg.h"


// CChatZone 对话框

IMPLEMENT_DYNAMIC(CChatDlg, CDialogEx)

CChatDlg::CChatDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CHATZONE, pParent)
	
{

}

CChatDlg::~CChatDlg()
{
}

void CChatDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FINDFRIEND, m_EditFriendName);
	//  DDX_Text(pDX, IDC_STATIC_CURUSER, m_currentAccount);
	DDX_Control(pDX, IDC_EDIT_CHAT, m_EditChatInput);
	DDX_Control(pDX, IDC_EDIT_RECORD, m_EditChatRecord);
	DDX_Control(pDX, IDC_EDIT_CURUSR, m_EditCurrentAccount);
	DDX_Control(pDX, IDC_LIST_FRIENDLIST, m_ListFriendList);
}


BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CChatDlg::OnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_CLEAN, &CChatDlg::OnClickedButtonClean)
	ON_BN_CLICKED(IDC_BUTTON_ADDFRIEND, &CChatDlg::OnClickedButtonAddfriend)
	ON_BN_CLICKED(IDC_BUTTON_RENEWLIST, &CChatDlg::OnClickedButtonRenewlist)
END_MESSAGE_MAP()


// CChatZone 消息处理程序





BOOL CChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// 获取本窗口句柄
	g_hWndChat = this->m_hWnd;

	// 获取聊天记录框的控件句柄
	g_pEditChatRecord = (CEdit*)GetDlgItem(IDC_EDIT_RECORD);

	//获取好友列表的列表控件句柄
	CListCtrl *pFriendList = (CListCtrl *)GetDlgItem(IDC_LIST_FRIENDLIST);
	g_pListFriendList = pFriendList;

	// 好友列表初始化（设置风格、插入列
	pFriendList->SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	pFriendList->InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	pFriendList->InsertColumn(1, "姓名", LVCFMT_CENTER, 80);


	// 搜索好友编辑框的默认值
	CEdit*  pEditFindFri = (CEdit*)GetDlgItem(IDC_EDIT_FINDFRIEND);//获取相应的编辑框ID
	pEditFindFri->SetWindowText(_T("请输入好友姓名")); //设置默认显示的内容 

	// 新开一个线程，接收消息
	CreateThread(0, 0, recvMessageProc, &g_pClient, 0, 0);

	// 显示当前账号
	CEdit*  pEditCurUsr = (CEdit*)GetDlgItem(IDC_EDIT_CURUSR);//获取相应的编辑框ID
	pEditCurUsr->SetWindowText(g_CurAccount);

	/*UpdateData(TRUE);
	char buff[30];
	sprintf_s(buff, " %s", g_CurAccount);
	m_currentAccount = buff;
	UpdateData(FALSE);
	*/


	// 获取好友列表
	GetFriendList(g_pClient);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CChatDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	// 关闭聊天框后再显示登录框
	AfxGetApp()->GetMainWnd()->ShowWindow(SW_NORMAL);
	//g_isLogin = false;
	//CWnd* phWnd = AfxGetApp()->GetMainWnd(); //获取主窗口的句柄
	//ShowWindow(SW_NORMAL);// 隐藏主窗口（当关闭会话框时再显示

	CDialogEx::OnClose();
}


void CChatDlg::OnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码

	// 获取发送的消息内容
	CString message;
	m_EditChatInput.GetWindowText(message);
	// 判断是否为空
	if (message.IsEmpty())
	{
		MessageBox("什么都不写，你发nima呢");
	}
	// 不为空则发送
	else
	{
		// 获取接收方并发送消息（好友&&选中状态）
		for (int i = 0; i < m_ListFriendList.GetItemCount(); i++)// 遍历所有好友
		{
			// 如果是选中状态就发送
			if (m_ListFriendList.GetCheck(i))
			{
				// 发送消息
				//SendMultiMsg(g_pClient, message);//广播
				CString sendToWho = m_ListFriendList.GetItemText(i, 1);
				SendMsg(g_pClient, message, g_CurAccount, sendToWho);// 单播、多播

				// 新+旧+时间，再显示在聊天记录
				CEdit*  pEditRecord = (CEdit*)GetDlgItem(IDC_EDIT_RECORD);
				CString strTime = CTime::GetCurrentTime() .Format("%Y-%m-%d %X   ");
				CString oldRecord;
				pEditRecord->GetWindowText(oldRecord);
				pEditRecord->SetWindowText(oldRecord + "\r\n"
					+ strTime + "[Send]" + "\r\n"
					+ ">  " + message 
					+ " [to: " + sendToWho + "]");

			}
		}




	}
		
}


void CChatDlg::OnClickedButtonClean()
{
	// TODO: 在此添加控件通知处理程序代码

	// 清空消息
	CEdit*  pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHAT);//获取相应的编辑框ID
	pEdit->SetWindowText(_T("")); //设置默认显示的内容 

}



void CChatDlg::OnClickedButtonAddfriend()
{
	// TODO: 在此添加控件通知处理程序代码

	// 获取好友名字
	CString friendName;
	m_EditFriendName.GetWindowText(friendName);

	//// CString转const char *
	//size_t  i;
	//const wchar_t* wstr = (LPCTSTR)friendName;
	//char sfriendName[20] = { 0 };
	//setlocale(LC_ALL, "chs");
	//wcstombs_s(&i, sfriendName, wstr, wcslen(wstr));
	//const char * ssfriendName = sfriendName;//不加则插入数据库失败
	//setlocale(LC_ALL, "C");

	// 判断内容是否为空
	if (friendName.IsEmpty())
		MessageBox("不可为空");
	else
	{
		AddFriend(g_pClient, friendName);
	}
}

void CChatDlg::OnClickedButtonRenewlist()
{
	// TODO: 在此添加控件通知处理程序代码
	GetFriendList(g_pClient);
}
