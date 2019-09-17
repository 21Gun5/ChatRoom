#pragma once


// CChatZone 对话框

class CChatDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CChatDlg)

public:
	CChatDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CChatDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHATZONE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_EditChatInput;
	CEdit m_EditChatRecord;
	CEdit m_EditCurrentAccount;
	CEdit m_EditFriendName;

	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnClickedButtonSend();
	afx_msg void OnClickedButtonClean();

//	afx_msg void OnUpdateEditChat();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
//	int RecvMessage(LPVOID arg);
//	CString m_currentAccount;

	
	afx_msg void OnClickedButtonAddfriend();
//	afx_msg void OnClickedButtonGetfriendlist();

	CListCtrl m_ListFriendList;
};
