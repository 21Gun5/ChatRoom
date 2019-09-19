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
	CEdit m_EditAddFriend;
	CListCtrl m_ListFriendList;

	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnClickedButtonSend();
	afx_msg void OnClickedButtonClean();
	afx_msg void OnClickedButtonAddfriend();
	afx_msg void OnClickedButtonF5friendlist();
	CEdit m_EditCreateRoom;
	afx_msg void OnClickedButtonCreateroom();
	CEdit m_EditJoinRoom;
	afx_msg void OnClickedButtonJoinroom();
	CListCtrl m_ListRoomList;
	afx_msg void OnClickedButtonF5roomlist();
};
