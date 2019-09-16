// CChatZone.cpp: 实现文件
//

#include "stdafx.h"
#include "ChatRoomClient.h"
#include "CChatZone.h"
#include "afxdialogex.h"
#include "client.h"
#include "ChatRoomClientDlg.h"


// CChatZone 对话框

IMPLEMENT_DYNAMIC(CChatZone, CDialogEx)

CChatZone::CChatZone(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CHATZONE, pParent)
	, m_currentAccount(_T(""))
{

}

CChatZone::~CChatZone()
{
}

void CChatZone::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FINDFRIEND, m_EditFindFriend);
	DDX_Text(pDX, IDC_STATIC_CURUSER, m_currentAccount);
	DDX_Control(pDX, IDC_EDIT_CHAT, m_EditChatInput);
	DDX_Control(pDX, IDC_EDIT_RECODE, m_EditChatRecord);
}


BEGIN_MESSAGE_MAP(CChatZone, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CChatZone::OnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_CLEAN, &CChatZone::OnClickedButtonClean)
//	ON_EN_UPDATE(IDC_EDIT_CHAT, &CChatZone::OnUpdateEditChat)
//ON_WM_TIMER()
END_MESSAGE_MAP()


// CChatZone 消息处理程序


void CChatZone::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值


	// 关闭聊天框后再显示登录框
	AfxGetApp()->GetMainWnd()->ShowWindow(SW_NORMAL);
	//g_isLogin = false;
	//CWnd* phWnd = AfxGetApp()->GetMainWnd(); //获取主窗口的句柄
	//ShowWindow(SW_NORMAL);// 隐藏主窗口（当关闭会话框时再显示

	CDialogEx::OnClose();
}


BOOL CChatZone::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// 搜索好友编辑框的默认值
	CEdit*  pEditFindFri = (CEdit*)GetDlgItem(IDC_EDIT_FINDFRIEND);//获取相应的编辑框ID
	pEditFindFri->SetWindowText(_T("请输入好友姓名")); //设置默认显示的内容 

	// 显示当前账号（暂未设置
	UpdateData(TRUE);
	m_currentAccount = "cur_user: not set";
	UpdateData(FALSE);


	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CChatZone::OnClickedButtonSend()
{
	// TODO: 在此添加控件通知处理程序代码

	// 获取发送的消息内容
	CString message;
	m_EditChatInput.GetWindowTextW(message);
	// 判断是否为空
	if (message.IsEmpty())
		MessageBox(L"什么都不写，你发nima呢");
	else
	{
		//MessageBox(CString("已发送: ") + message);

		// cstring 转const char *
		size_t  i;
		const wchar_t* wstr = (LPCTSTR)message;
		char smessage[20] = { 0 };
		setlocale(LC_ALL, "chs");
		wcstombs_s(&i, smessage, wstr, wcslen(wstr));
		const char * ssmessage = smessage;//不加则插入数据库失败
		setlocale(LC_ALL, "C");
		// 发送消息
		SendMsg(g_pClient, ssmessage);

		// 显示在聊天记录


		CString strTime; //获取系统时间 　　
		CTime tm;
		tm = CTime::GetCurrentTime();
		strTime = tm.Format("%Y-%m-%d %X   ");


		CEdit*  pEditRecord = (CEdit*)GetDlgItem(IDC_EDIT_RECODE);//获取相应的编辑框ID
		pEditRecord->SetWindowText(strTime+"Send:   "+message); //设置默认显示的内容
	}
		
}


void CChatZone::OnClickedButtonClean()
{
	// TODO: 在此添加控件通知处理程序代码

	// 清空消息
	CEdit*  pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CHAT);//获取相应的编辑框ID
	pEdit->SetWindowText(_T("")); //设置默认显示的内容 

}


//void CChatZone::OnUpdateEditChat()
//{
//	// TODO:  如果该控件是 RICHEDIT 控件，它将不
//	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
//	// 函数，以将 EM_SETEVENTMASK 消息发送到该控件，
//	// 同时将 ENM_UPDATE 标志“或”运算到 lParam 掩码中。
//
//
//	 
//
//	// TODO:  在此添加控件通知处理程序代码
//
//	CString message;
//	m_EditChatInput.GetWindowTextW(message);
//
//	CEdit*  pEditFindFri = (CEdit*)GetDlgItem(IDC_EDIT_FINDFRIEND);//获取相应的编辑框ID
//	pEditFindFri->SetWindowText(_T("请输入好友姓名")); //设置默认显示的内容
//}


//void CChatZone::OnTimer(UINT_PTR nIDEvent)
//{
//	// TODO: 在此添加消息处理程序代码和/或调用默认值
//
//	CDialogEx::OnTimer(nIDEvent);
//}
