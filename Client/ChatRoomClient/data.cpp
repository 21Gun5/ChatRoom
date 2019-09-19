#include "stdafx.h"
#include "data.h"

// 全局变量
bool g_isLogin = false;// 是否登录
char g_recvMessage[200] = { 0 };// 接收到的消息
HWND g_hWndChat = NULL;
CEdit * g_pEditChatRecord = NULL;
CListCtrl * g_pListFriendList = NULL;
CListCtrl * g_pListRoomList = NULL;


