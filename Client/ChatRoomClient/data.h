#pragma once

// ȫ�ֱ���
extern bool g_isLogin;
extern char g_recvMessage[200];
extern HWND  g_hWndChat;
extern CEdit * g_pEditChatRecord;
extern CListCtrl * g_pListFriendList;
extern CListCtrl * g_pListRoomList;
// ���ݰ�����
enum DataPackType 
{
	login = 1,//��¼����
	registe = 2,//ע��
	sendRoomMsg = 3,// �㲥
	sendMsg = 4,//����
	addFriend = 5,// �Ӻ���
	getFriendList = 6,//��ȡ�����б�
	createRoom = 7,// ��������
	joinRoom = 8,//���뷿��
	getroomlist = 9,//��ȡ�����б�
	getroommember = 10//�õ�Ⱥ��Ա
};
// �������ݵĽṹ
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