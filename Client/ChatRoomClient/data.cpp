#include "stdafx.h"
#include "data.h"

// ȫ�ֱ���
bool g_isLogin = false;// �Ƿ��¼
char g_recvMessage[200] = { 0 };// ���յ�����Ϣ
HWND g_hWndChat = NULL;
CEdit * g_pEditChatRecord = NULL;
CListCtrl * g_pListFriendList = NULL;
CListCtrl * g_pListRoomList = NULL;


