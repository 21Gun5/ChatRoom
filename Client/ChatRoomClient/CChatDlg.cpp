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
	DDX_Control(pDX, IDC_EDIT_ADDFRIEND, m_EditAddFriend);
	DDX_Control(pDX, IDC_EDIT_CHAT, m_EditChatInput);
	DDX_Control(pDX, IDC_EDIT_RECORD, m_EditChatRecord);
	DDX_Control(pDX, IDC_EDIT_CURUSR, m_EditCurrentAccount);
	DDX_Control(pDX, IDC_LIST_FRIENDLIST, m_ListFriendList);
	DDX_Control(pDX, IDC_EDIT_CREATEROOM, m_EditCreateRoom);
	DDX_Control(pDX, IDC_EDIT_JOINROOM, m_EditJoinRoom);
	DDX_Control(pDX, IDC_LIST_ROOMLIST, m_ListRoomList);
}

BEGIN_MESSAGE_MAP(CChatDlg, CDialogEx)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CChatDlg::OnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_CLEAN, &CChatDlg::OnClickedButtonClean)
	ON_BN_CLICKED(IDC_BUTTON_ADDFRIEND, &CChatDlg::OnClickedButtonAddfriend)
	ON_BN_CLICKED(IDC_BUTTON_F5FRIENDLIST, &CChatDlg::OnClickedButtonF5friendlist)
	ON_BN_CLICKED(IDC_BUTTON_CREATEROOM, &CChatDlg::OnClickedButtonCreateroom)
	ON_BN_CLICKED(IDC_BUTTON_JOINROOM, &CChatDlg::OnClickedButtonJoinroom)
	ON_BN_CLICKED(IDC_BUTTON_F5ROOMLIST, &CChatDlg::OnClickedButtonF5roomlist)
END_MESSAGE_MAP()

// CChatZone 消息处理程序

BOOL CChatDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化

	// 设置全局变量
	g_hWndChat = this->m_hWnd;//本窗口句柄
	g_pEditChatRecord = (CEdit*)GetDlgItem(IDC_EDIT_RECORD);//聊天记录框的控件句柄
	g_pListFriendList = (CListCtrl *)GetDlgItem(IDC_LIST_FRIENDLIST);//好友列表的列表控件句柄
	g_pListRoomList = (CListCtrl *)GetDlgItem(IDC_LIST_ROOMLIST);//群列表的列表控件句柄

	// 好友列表初始化（设置风格、插入列
	g_pListFriendList->SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	g_pListFriendList->InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	g_pListFriendList->InsertColumn(1, "姓名", LVCFMT_CENTER, 80);
	// 群列表初始化
	g_pListRoomList->SetExtendedStyle(LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	g_pListRoomList->InsertColumn(0, "序号", LVCFMT_CENTER, 40);
	g_pListRoomList->InsertColumn(1, "群名", LVCFMT_CENTER, 80);

	// 设置编辑框的默认值
	CEdit*  pEditAddFriend = (CEdit*)GetDlgItem(IDC_EDIT_ADDFRIEND);
	pEditAddFriend->SetWindowText(_T("请输入好友姓名"));
	CEdit*  pEditCreateRoom = (CEdit*)GetDlgItem(IDC_EDIT_CREATEROOM);
	pEditCreateRoom->SetWindowText(_T("请输入群名称"));
	CEdit*  pEditJoinRoom = (CEdit*)GetDlgItem(IDC_EDIT_JOINROOM);
	pEditJoinRoom->SetWindowText(_T("请输入群名称"));

	// 新开一个线程，接收消息
	CreateThread(0, 0, recvMessageProc, &g_pClient, 0, 0);

	// 显示当前账号
	CEdit*  pEditCurUsr = (CEdit*)GetDlgItem(IDC_EDIT_CURUSR);//获取相应的编辑框ID
	pEditCurUsr->SetWindowText(g_CurAccount);

	// 获取好友列表、群列表
	GetFriendList(g_pClient);
	GetRoomList(g_pClient);

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
		// 获取接收好友并发送消息（好友&&选中状态）
		for (int i = 0; i < m_ListFriendList.GetItemCount(); i++)// 遍历所有好友
		{
			// 如果是选中状态就发送
			if (m_ListFriendList.GetCheck(i))
			{
				// 发送消息
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

		// 获取接收群并发送消息
		for (int i = 0; i < m_ListRoomList.GetItemCount(); i++)// 遍历所有群
		{
			// 如果是选中状态就发送
			if (m_ListRoomList.GetCheck(i))
			{
				// 发送消息
				//SendMultiMsg(g_pClient, message);//广播
				CString sendToWho = m_ListRoomList.GetItemText(i, 1);
				SendRoomMsg(g_pClient, message, g_CurAccount, sendToWho);// 单播、多播

				// 新+旧+时间，再显示在聊天记录
				CEdit*  pEditRecord = (CEdit*)GetDlgItem(IDC_EDIT_RECORD);
				CString strTime = CTime::GetCurrentTime().Format("%Y-%m-%d %X   ");
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
	m_EditAddFriend.GetWindowText(friendName);
	// 判断内容是否为空
	if (friendName.IsEmpty())
		MessageBox("不可为空");
	else
		AddFriend(g_pClient, friendName);
}
void CChatDlg::OnClickedButtonF5friendlist()
{
	// TODO: 在此添加控件通知处理程序代码
	GetFriendList(g_pClient);
}
void CChatDlg::OnClickedButtonCreateroom()
{
	// TODO: 在此添加控件通知处理程序代码

	// 获取群名字
	CString roomName;
	m_EditCreateRoom.GetWindowText(roomName);
	// 判断内容是否为空
	if (roomName.IsEmpty())
		MessageBox("不可为空");
	else
		CreateRoom(g_pClient, roomName);
}
void CChatDlg::OnClickedButtonJoinroom()
{
	// TODO: 在此添加控件通知处理程序代码

	// 获取群名字
	CString roomName;
	m_EditJoinRoom.GetWindowText(roomName);
	// 判断内容是否为空
	if (roomName.IsEmpty())
		MessageBox("不可为空");
	else
		JoinRoom(g_pClient, roomName);
}
void CChatDlg::OnClickedButtonF5roomlist()
{
	// TODO: 在此添加控件通知处理程序代码
	GetRoomList(g_pClient);
}
