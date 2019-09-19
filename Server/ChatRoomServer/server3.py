import hashlib
import socket
import struct
import threading
import pymysql
import base64
from enum import Enum

# 请求类型枚举
class Request(Enum):
    login = 1           # 登录
    register = 2        # 注册
    sendRoomMsg = 3     # 群聊
    sendMsg = 4         # 单聊及多聊
    addFriend = 5       # 加好友
    getFriendList = 6   # 获取好友列表
    createRoom = 7      # 创建群聊
    joinRoom = 8        # 加入群聊
    getRoomList = 9     # 获取群聊列表
    getRoomMember = 10  # 获取群聊成员
# 响应结果枚举
class ResultCode(Enum):
    notify = -1         # 服务端主动发通知
    success = 0         # 成功
    bad_pack = 1        # 数据包错误
    user_exist = 2      # 用户已存在
    user_non_exist = 3  # 用户不在线
    password_wrong = 4  # 密码错误
    room_exist = 5      # 群已存在
    already_in_room = 6 # 已在群里
# 连接数据库
class Database:
    def __init__(self):
        self.connecter = pymysql.connect(host='127.0.0.1',
                                         user='root',
                                         password='niw123',
                                         db='chatroom',
                                         charset='utf8',
                                         cursorclass=pymysql.cursors.DictCursor)
        self.cursor = self.connecter.cursor(cursor=pymysql.cursors.DictCursor)
# 处理各种请求
class Handler():
    def __init__(self):
        self.args = None    # 客户端传来的参数
        self.socket = None  # 客户端socket
        self.account = ''   # 客户端用户账号
        self.id = 0
    def register(self):
        # 提取参数
        print('注册 参数: ', self.args)
        username = self.args[0]
        password = self.args[1]  # here
        # password = md5(password)    # 加密密码
        # 账号是否已注册
        sql = "select * from chatroom_user where account='%s';" % username
        _db.cursor.execute(sql) # 执行sql语句
        result = _db.cursor.fetchone()  # 获取结果集
        if result != None:
            sendToclientSocket(self.socket,Request.register.value,ResultCode.user_exist.value,"用户已存在")
            return
        #  账号及密码插入数据库
        sql = "insert into chatroom_user (account, passwd) VALUE('%s','%s');" % (username, password)
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')  # 提交到数据库
        # 结果反馈给客户端
        sendToclientSocket(self.socket,Request.register.value,ResultCode.success.value,"注册成功")
    def login(self):
        # 提取参数
        print('登陆 参数: ', self.args)
        username = self.args[0]
        password = self.args[1]
        # 账号是否存在
        sql = "select * from chatroom_user where account='%s';" % username
        _db.cursor.execute(sql)
        user_info = _db.cursor.fetchone()    # 获取结果集（字典：键-字段名, 值-值
        if user_info == None:
            sendToclientSocket(self.socket,Request.login.value,ResultCode.user_non_exist.value,"用户名不存在")
            return
        #  密码是否正确
        if user_info['passwd'] != password:  # here
            # if userInfo['passwd'] != md5(password):
            sendToclientSocket(self.socket,Request.login.value,ResultCode.password_wrong.value,"密码错误")
            return
        sendToclientSocket(self.socket,Request.login.value,ResultCode.success.value,"登陆成功")
        # 将已登陆用户保存到在线用户字典
        self.__dict__.update(user_info)
        _clientDict[self.account] = self
    def sendRoomMsg(self):
        # 提取参数
        print('群聊消息 参数: ', self.args)
        msg = self.args[0]          # 消息
        recv_from = self.args[1]    # 发送方
        send_to = self.args[2]      # 接收方
        # 获取群内的所有成员
        sql = "select * from chatroom_room where account='%s';" % send_to
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if not room:
            sendToclientSocket(self.socket,Request.getRoomMember.value,ResultCode.room_exist.value,"此群不存在")
            return
        sql = "select * from chatroom_user where id in(" \
              "select id_user from chatroom_roommember where id_room=%d);" % (room['id'])   # 通过外键
        _db.cursor.execute(sql)
        all_member = _db.cursor.fetchall()
        # 遍历群成员
        for u in all_member:
            # 跳过自己
            if u['account'] == self.account:
                continue
            # 若在线则发送
            if u['account'] in _clientDict:  # 其是否在线
                sendToclientSocket(_clientDict[u['account']].socket,Request.sendRoomMsg.value,ResultCode.notify.value,
                                   msg + ' [from: ' + send_to + '群-' + recv_from + ']')
    def sendMsg(self):  # here
        # 提取参数
        print('单播消息 参数: ', self.args)
        msg = self.args[0]          # 消息
        recv_from = self.args[1]    # 发送方
        send_to = self.args[2]      # 接收方
        # 遍历在线用户
        for k, v in _clientDict.items():
            # 跳过自己
            if (k == self.account):
                continue
            # 只给指定的接收者
            if (k != send_to):
                continue
            sendToclientSocket(v.socket,Request.sendMsg.value,ResultCode.notify.value,
                               msg + ' [from: ' + recv_from + ']')
    def addFriend(self):
        # 提取参数
        print('加好友 参数: ', self.args)
        friend_name = self.args[0]
        type = self.args[1]
        print('添加好友请求, self = ', self.account)
        # 是否为本人
        if self.account == friend_name:
            sendToclientSocket(self.socket,Request.addFriend.value,ResultCode.user_non_exist.value,"为本人")
            return
        # 账号是否存在
        sql = "select * from chatroom_user where account='%s';" % friend_name
        _db.cursor.execute(sql)
        user = _db.cursor.fetchone()
        if user == None:
            sendToclientSocket(self.socket,Request.addFriend.value,ResultCode.user_non_exist.value,"用户不存在")
            return
        # 是否已是好友
        sql = "select * from chatroom_friend where user_1=%d and user_2=%d;" \
              % (self.id, user['id'])
        _db.cursor.execute(sql)
        if _db.cursor.fetchone() != None:
            sendToclientSocket(self.socket,Request.addFriend.value,ResultCode.success.value,"已是好友")
            return
        # 判断类型：0-请求、1-回复-同意、2-回复-拒绝
        if type == '0': # 申请加好友
            # 对方是否在线
            if user['account'] not in _clientDict:
                sendToclientSocket(self.socket,Request.addFriend.value,ResultCode.user_non_exist.value,"好友不在线")
                return
            # 发送给好友
            sendToclientSocket(_clientDict[user['account']].socket,Request.addFriend.value,ResultCode.notify.value,self.account)
            # 发送给申请者
            sendToclientSocket(self.socket,Request.addFriend.value,ResultCode.success.value,"等待对方响应")
            return
        elif type == '1':  # 同意加好友
            # 1. 得到同意添加为好友对的用户的账号,根据账号查找数据库得到用户信息
            sql = "select * from chatroom_user where account='%s';" % friend_name
            _db.cursor.execute(sql)
            user = _db.cursor.fetchone()
            # 好友关系插入数据库
            sql = "insert into chatroom_friend (user_1, user_2) VALUES (%d,%d),(%d,%d);" % \
                  (self.id, user['id'], user['id'], self.id)
            _db.cursor.execute(sql)
            _db.cursor.execute("commit;")
            # 3. 通知申请者，好友请求已通过
        elif type == '2':   # 拒绝加好友
            pass

    # def addFriend(self):
    #     print('添加好友请求, self = ', self.account)
    #
    #     # 添加好友
    #     # 客户端 -> 服务端, 参数: 好友的账号\n0|1|2
    #     #           0|1|2 : 0 表示这是一个添加好友的请求
    #     #                   1 表示这是一个同意被xxx添加为好友的回复(回复给服务端)
    #     #                   2 表示这是一个拒绝被xxx添加为好友的回复(回复给服务端)
    #     # 服务端 -> 客户端. 参数: 用户账号
    #
    #     # 1. 判断是不是自己
    #     if self.account == self.args[0]:
    #         sendToclientSocket(self.socket,
    #                            Request.addFriend.value,
    #                            ResultCode.user_non_exist.value,
    #                            "不能添加自己")
    #         return
    #     # 1.1 判断要添加的用户账号是否存在
    #     sql = "select * from chatroom_user where account='%s';" % (self.args[0])
    #     _db.cursor.execute(sql)
    #     user = _db.cursor.fetchone()
    #     if user == None:
    #         sendToclientSocket(self.socket,
    #                            Request.addFriend.value,
    #                            ResultCode.user_non_exist.value,
    #                            "要添加的用户不存在")
    #         return
    #
    #     # 2. 判断是否已经添加过了
    #     sql = "select * from chatroom_friend where user_1=%d and user_2=%d;" \
    #           % (self.id, user['id'])
    #     _db.cursor.execute(sql)
    #     # 获取查询的结果集,如果啥都没查到就返回None,否则就不会是None
    #     if _db.cursor.fetchone() != None:
    #         sendToclientSocket(self.socket,
    #                            Request.addFriend.value,
    #                            ResultCode.success.value,
    #                            "已添加过了")
    #         return
    #
    #     # 判断是添加好友的请求还是回复. 0是请求, 1是回复(同意), 2是回复(拒绝)
    #     if self.args[1] == '0':
    #         # 3. 等待对方同意之后,再将两者的关系写入到好友关系表中.
    #         if user['account'] not in _clientDict:
    #             sendToclientSocket(self.socket,
    #                                Request.addFriend.value,
    #                                ResultCode.user_non_exist.value,
    #                                "好友不在线")
    #             return
    #         # 将添加好友的申请发送给目标客户端
    #         sendToclientSocket(_clientDict[user['account']].socket,  # 目标的套接字
    #                            Request.addFriend.value,
    #                            ResultCode.notify.value,
    #                            self.account)
    #
    #         # 将结果发回给发送了好友请求的客户端
    #         sendToclientSocket(self.socket,
    #                            Request.addFriend.value,
    #                            ResultCode.success.value,
    #                            "发送成功,等待对方接受")
    #         return
    #     elif self.args[1] == '1':  # 表示同意用户的添加好友请求, 此时可以将两者的关系写入到好友表
    #         # 1. 得到同意添加为好友对的用户的账号,根据账号查找数据库得到用户信息
    #         sql = "select * from chatroom_user where account='%s';" % (self.args[0])
    #         _db.cursor.execute(sql)
    #         user = _db.cursor.fetchone()
    #         # 2. 插入数据库
    #         sql = "insert into chatroom_friend (user_1, user_2) VALUES (%d,%d),(%d,%d);" % \
    #               (self.id, user['id'], user['id'], self.id)
    #         _db.cursor.execute(sql)
    #         _db.cursor.execute("commit;")
    #         # 3. 可以通知对方,好友请求已经被同意了
    #
    #     elif self.args[1] == '2':
    #         pass
    #         # 拒绝添加好友
    #         # 可以通知对方,好友请求已经被拒绝了

    def getFriendList(self):
        # 查询数据库得到所有好友
        print('获取好友列表 参数: ', self.args)
        sql = "select * from chatroom_user where id in " \
              "(select user_2 from chatroom_friend where user_1 = %d);" % (self.id)
        _db.cursor.execute(sql)
        # 获取结果集
        all = _db.cursor.fetchall()
        friendList = []
        # 遍历结果集
        for f in all:
            friendList.append(f['account'])
        # 将所有好友的名字使用\n作为分隔符合并成字符串
        fList = '\n'.join(friendList)
        # 发送给客户端.
        sendToclientSocket(self.socket,
                           Request.getFriendList.value,
                           ResultCode.success.value,
                           fList)
    def createRoom(self):
        # 提取参数
        print('创建群 参数: ', self.args)
        roomName = self.args[0]
        # 群是否已创建
        sql = "select * from chatroom_room where account='%s';" % roomName
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if room:
            sendToclientSocket(self.socket,Request.createRoom.value,ResultCode.room_exist.value,"群已存在")
            return
        # 不存在则新建
        sql = "insert into chatroom_room (account) VALUES('%s');" % roomName
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')
        # 将创建者加入群，并设置为群主
        sql = "select * from chatroom_room where account='%s';" % roomName
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        sql = "insert into chatroom_roommember (id_user, id_room,user_status) " \
              " VALUES(%d,%d,0)" % (self.id, room['id'])
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')
        # 反馈到客户端
        sendToclientSocket(self.socket,Request.createRoom.value,ResultCode.success.value,"创建成功")
    def joinRoom(self):
        # 提取参数
        print('加入群 参数: ', self.args)
        roomName = self.args[0]
        # 群是否存在
        sql = "select * from chatroom_room where account='%s';" % (roomName)
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if not room:
            sendToclientSocket(self.socket,Request.joinRoom.value,ResultCode.room_exist.value,"群不存在")
            return
        # 是否已经加入
        sql = "select * from chatroom_roommember where " \
              "id_room=%d and id_user=%d;" % (room['id'], self.id)
        _db.cursor.execute(sql)
        if _db.cursor.fetchone():
            sendToclientSocket(self.socket,Request.joinRoom.value,ResultCode.already_in_room.value,"早就在群里了")
            return
        # 无条件加入（或申请群主
        sql = "insert into chatroom_roommember (id_user, id_room, user_status) " \
              "VALUES (%d,%d,2)" % (self.id, room['id'])
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')
        sendToclientSocket(self.socket,Request.joinRoom.value,ResultCode.success.value,"加入成功")
    def getRoomList(self):
        # 提取参数
        print('获取群列表 参数: ', self.args)
        # 获取群名，并加入列表
        room_name_list = []
        sql = "select * from chatroom_room where id in( " \
              " select id_room from chatroom_roommember where id_user=%d);" % (self.id)
        _db.cursor.execute(sql)
        all = _db.cursor.fetchall()
        for r in all:
            room_name_list.append(r['account'])
        # 将列表转为字符串发送给客户端
        names = '\n'.join(room_name_list)
        sendToclientSocket(self.socket,Request.getRoomList.value,ResultCode.success.value,names)
    def getRoomMember(self):
        # 提取参数
        print('获取群成员 参数: ', self.args)
        roomName = self.args[0]
        # 群是否存在
        sql = "select * from chatroom_room where account='%s';" % (roomName)
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if not room:
            sendToclientSocket(self.socket,Request.getRoomMember.value,ResultCode.room_exist.value,"此群不存在")
            return
        # 获取成员并加入列表
        friend_list = []
        sql = "select * from chatroom_user where id in(" \
              "select id_user from chatroom_roommember where id_room=%d);" % (room['id'])
        _db.cursor.execute(sql)
        all = _db.cursor.fetchall()
        for f in all:
            friend_list.append(f['account'])
        # 将列表转为字符串发送给客户端
        names = '\n'.join(friend_list)
        sendToclientSocket(self.socket,Request.getRoomMember.value,ResultCode.success.value,names)
# 服务器
class Server:
    def __init__(self, ip, port):
        self.socket = socket.socket()           # 创建套接字
        self.socket.bind((ip, port))            # 绑定地址
        self.socket.listen(socket.SOMAXCONN)    # 开启监听
    def recvclientSocketData(self, handler):
        type, args = None, None
        while True:
            type, args = None, None
            try:
                type, args = recvFromclientSocket(handler.socket)   # 接收消息
            except Exception as e:
                del _clientDict[handler.account]    # 从在线用户中删除
                break
            try:
                type = Request(type)   # 转换成枚举类型
            except:
                sendToclientSocket(handler.socket,
                                   type,
                                   ResultCode.bad_pack.value,
                                   "无效的请求")
            #  获取相应的处理函数
            func = getattr(handler, type.name)
            handler.args = args
            func()
    def run(self):
        while True:
            # 收到客户端连接
            clientSocket, addr = self.socket.accept()
            # 创建处理请求对象并设置
            h = Handler()
            h.socket = clientSocket
            h.addr = addr
            print('客户端上线: ', addr)
            # 开启新线程，来接收消息
            threading.Thread(target=self.recvclientSocketData,args=(h,)).start()

# 全局变量
_db = Database()    # 数据库对象
_clientDict = {}    # 在线用户

# md5加密
def md5(s):
    hashObj = hashlib.md5()
    hashObj.update(s.encode('gb2312'))
    return hashObj.hexdigest()
# 收发数据
def sendToclientSocket(clientSocket, type, status, strData):
    data = strData.encode('gb2312')
    # 发之间先加密（加密盒子里的数据，而不是盒子本身）
    data = base64.b64encode(data)  # here

    size = len(data)
    strFormat = 'iii%ds' % (size)
    rawData = struct.pack(strFormat, type, status, size, data)
    clientSocket.send(rawData)
def recvFromclientSocket(clientSocket):
    data = clientSocket.recv(8)
    if len(data) == 0:
        raise Exception('客户端断开连接')
    # 接收8个字节的头部.
    type, size = struct.unpack('ii', data)
    print('type=%d , size%d' % (type, size))

    # 接收后续的内容
    bodyData = clientSocket.recv(size)
    # 接收后先解密
    bodyData = base64.b64decode(bodyData)  # here

    print('bodyData: %s' % (bodyData.decode('gb2312')))
    # 解码之后分割参数
    return type, bodyData.decode('gb2312').split('\n')

if __name__ == '__main__':
    s = Server('127.0.0.1', 10086)
    s.run()
