#pragma once

// 全局变量
extern bool g_isLogin;
extern char g_recvMessage[200];
extern HWND  g_hWndChat;
extern CEdit * g_pEditChatRecord;
extern CListCtrl * g_pListFriendList;
extern CListCtrl * g_pListRoomList;
// 数据包类型
enum DataPackType 
{
	login = 1,//登录类型
	registe = 2,//注册
	sendRoomMsg = 3,// 广播
	sendMsg = 4,//单播
	addFriend = 5,// 加好友
	getFriendList = 6,//获取好友列表
	createRoom = 7,// 创建房间
	joinRoom = 8,//加入房间
	getroomlist = 9,//获取房间列表
	getroommember = 10//得到群成员
};
// 网络数据的结构
#pragma pack(push,1)
struct DataPack
{
	DataPackType type;
	unsigned int size;
};
struct DataPackResult {
	unsigned int type;
	unsigned int status;
	unsigned int size;
	char data[1];
};
#pragma pack(pop)