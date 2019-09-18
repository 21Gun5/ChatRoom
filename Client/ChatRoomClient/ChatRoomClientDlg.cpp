
// ChatRoomClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "ChatRoomClient.h"
#include "ChatRoomClientDlg.h"
#include "afxdialogex.h"

//#include "client.h"
//#include "data.h"
#include <string>
#include "CChatDlg.h"


Client client("127.0.0.1", 10086);
Client * g_pClient = &client;
CString g_CurAccount = NULL;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CChatRoomClientDlg 对话框



CChatRoomClientDlg::CChatRoomClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHATROOMCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


void CChatRoomClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_USR, m_EditUserName);
	DDX_Control(pDX, IDC_EDIT_PWD, m_EditPassWord);
}

BEGIN_MESSAGE_MAP(CChatRoomClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CChatRoomClientDlg::OnClickedButtonRegister)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CChatRoomClientDlg::OnClickedButtonLogin)
END_MESSAGE_MAP()


// CChatRoomClientDlg 消息处理程序

BOOL CChatRoomClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	
	// 创建线程来注册/登录
	CreateThread(0, 0, recvLoginProc, &client, 0, 0);

	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CChatRoomClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CChatRoomClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标显示。
HCURSOR CChatRoomClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CChatRoomClientDlg::OnClickedButtonRegister()
{
	// TODO: 在此添加控件通知处理程序代码
	CString userName, passWord;

	// 获取密码和用户框的内容
	m_EditUserName.GetWindowText(userName);
	m_EditPassWord.GetWindowText(passWord);

	// CString转const char *
	//const char * suserName = (LPCSTR)(LPCTSTR)userName;
	//const char * spassWord = (LPCSTR)(LPCTSTR)passWord;//错误，只保留一个字符
	//size_t  i;
	//const wchar_t* wstr = (LPCTSTR)userName;
	//const wchar_t* wstr2 = (LPCTSTR)passWord;
	//char suserName[20] = { 0 };
	//char spassWord[20] = { 0 };
	//setlocale(LC_ALL, "chs");
	//wcstombs_s(&i, suserName, wstr, wcslen(wstr));
	//const char * ssuserName = suserName;//不加则插入数据库失败
	//const char * sspassWord = spassWord;
	//wcstombs_s(&i, spassWord, wstr2, wcslen(wstr2));
	//setlocale(LC_ALL, "C");

	// 判断内容是否为空
	if (userName.IsEmpty() || passWord.IsEmpty())
		MessageBox("请输入用户名和密码");
	else
	{
		Register(&client,userName,passWord);
		//MessageBox(CString("注册成功 \n用户名: ") + userName+ "\n" + CString("密   码: ") + passWord);
	}
}

void CChatRoomClientDlg::OnClickedButtonLogin()
{
	// TODO: 在此添加控件通知处理程序代码
	CString userName, passWord;

	// 获取密码和用户框的内容
	m_EditUserName.GetWindowText(userName);
	m_EditPassWord.GetWindowText(passWord);

	//// CString转const char *
	//size_t  i;
	//const wchar_t* wstr = (LPCTSTR)userName;
	//const wchar_t* wstr2 = (LPCTSTR)passWord;
	//char suserName[20] = { 0 };
	//char spassWord[20] = { 0 };
	//setlocale(LC_ALL, "chs");
	//wcstombs_s(&i, suserName, wstr, wcslen(wstr));
	//const char * ssuserName = suserName;//不加则插入数据库失败
	//const char * sspassWord = spassWord;
	//wcstombs_s(&i, spassWord, wstr2, wcslen(wstr2));
	//setlocale(LC_ALL, "C");

	// 判断内容是否为空
	if (userName.IsEmpty() || passWord.IsEmpty())
		MessageBox("不可为空");
	else
	{
		Login(&client,userName,passWord);
		Sleep(1000);// 等会儿另一个线程
		if (g_isLogin)
		{
			g_CurAccount = userName;// 设置当前用户

			CChatDlg chatZoneDlg(this);
			chatZoneDlg.DoModal();
			//// 新开一个线程，接收消息
			//CreateThread(0, 0, recvProc2, &g_pClient, 0, 0);
		}

	}
}
