#pragma once


// CChatZone 对话框

class CChatZone : public CDialogEx
{
	DECLARE_DYNAMIC(CChatZone)

public:
	CChatZone(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CChatZone();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CHATZONE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	CEdit m_EditFindFriend;
	virtual BOOL OnInitDialog();
	CString m_currentAccount;
	CEdit m_EditChatInput;
	afx_msg void OnClickedButtonSend();
	afx_msg void OnClickedButtonClean();
//	afx_msg void OnUpdateEditChat();
	CEdit m_EditChatRecord;
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
