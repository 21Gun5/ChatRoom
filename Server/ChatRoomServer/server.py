import hashlib
import random
import socket
import struct
import threading
from enum import Enum

import pymysql


class Database:
    def __init__(self):
        self.connecter = pymysql.connect(host='127.0.0.1',
                                         user='root',
                                         password='niw123',# here
                                         db='chatroom',
                                         charset='utf8',
                                         cursorclass=pymysql.cursors.DictCursor)
        self.cursor = self.connecter.cursor(cursor=pymysql.cursors.DictCursor)


# 构造一个全局的数据库对象
_db = Database()


class Request(Enum):
    # 登陆请求
    # 客户端 -> 服务端, 参数: 账号\n密码
    login = 1
    # 注册请求
    # 客户端 -> 服务端, 参数: 账号\n密码
    register = 2
    # 发消息请求 客户端 -> 服务端, 参数: 消息
    sendMultiMsg = 3
    # 发消息给个人
    # 客户端 -> 服务端, 参数: 用户账号\n发送的消息(不能有\n)
    # 服务度 -> 客户端, 参数: 用户账号\n发送的消息(不能有\n)
    sendMsg = 4

    # 添加好友
    # 客户端 -> 服务端, 参数: 好友的账号\n0|1|2
    #           0|1|2 : 0 表示这是一个添加好友的请求
    #                   1 表示这是一个接受被xxx添加为好友的回复(回复给服务端)
    #                   2 表示这是一个拒绝被xxx添加为好友的回复(回复给服务端)
    # 服务端 -> 客户端. 参数: 发起好友请求的账号
    addFriend = 5

    # 获取好友列表
    # 服务端 -> 客户端, 参数: 好友1账号\n好友2账号\n好友n账号
    getFriendList = 6


class ResultCode(Enum):
    notify = -1  # 表示消息是由服务端主动发起的通知
    success = 0  # 处理成功
    bad_pack = 1  # 不正确的数据包
    user_exist = 2  # 用户已存在
    user_non_exist = 3  # 用户不存在
    password_wrong = 4  # 密码错误


def md5(s):
    hashObj = hashlib.md5()
    hashObj.update(s.encode('gb2312'))
    return hashObj.hexdigest()


class Handler():
    # 客户端请求的处理类
    def __init__(self):
        self.args = None
        self.socket = None
        self.account = ''
        self.id = 0

    def register(self):
        # 注册
        print('注册 参数: ', self.args)
        # 注册的流程
        # 1. 检查账号有没有注册过.
        sql = "select * from chatroom_user where account='%s';" % (self.args[0])
        # 执行sql语句
        _db.cursor.execute(sql)
        # 获取结果集
        result = _db.cursor.fetchone()
        if result != None:
            sendToclientSocket(self.socket,
                               Request.register.value,
                               ResultCode.user_exist.value,
                               "用户已存在")
            return
        # 2. 保存账号到数据库
        # passwd = md5(self.args[1])# here
        passwd = self.args[1]
        sql = "insert into chatroom_user (account, passwd) VALUE('%s','%s');" \
              % (self.args[0], passwd)
        # 执行sql语句
        _db.cursor.execute(sql)
        # 提交sql语句的更改
        _db.cursor.execute('commit;')  # 提交到数据库
        # 将结果发送给客户端
        sendToclientSocket(self.socket,
                           Request.register.value,
                           ResultCode.success.value,
                           "注册成功")

    def login(self):
        print('登陆 参数: ', self.args)
        # 1. 查表,判断用户是否存在
        sql = "select * from chatroom_user where account='%s';" % (self.args[0])
        _db.cursor.execute(sql)
        # 获取sql语句执行后的结果集,结果集是一个字典,键就是表的字段名, 值就是表中的值.
        userInfo = _db.cursor.fetchone()
        # 如果没有结果集, 函数会返回None
        if userInfo == None:
            # 如果在数据库找不到用户的信息,则说明用户未注册,将结果发回给客户端
            sendToclientSocket(self.socket,
                               Request.login.value,
                               ResultCode.user_non_exist.value,
                               "用户名不存在")
            return
        # 2. 比较密码是否正确
        # if userInfo['passwd'] != md5( self.args[1] ):# here
        if userInfo['passwd'] != self.args[1]:  # here
            sendToclientSocket(self.socket,
                               Request.login.value,
                               ResultCode.password_wrong.value,
                               "密码错误")
            return
        sendToclientSocket(self.socket,
                           Request.login.value,
                           ResultCode.success.value,
                           "登陆成功")

        # 将已登陆的用户信息保存到字典
        # self.user = userInfo
        self.__dict__.update(userInfo)
        _clientDict[self.account] = self

    def sendMultiMsg(self):
        print('广播消息 参数: ', self.args)
        msg = self.args[0]
        for k, v in _clientDict.items():
            if (k == self.account):
                continue
            sendToclientSocket(v.socket,
                               Request.sendMultiMsg.value,
                               ResultCode.notify.value,
                               msg)

    def sendMsg(self):
        print('单播消息 参数: ', self.args)
        msg = self.args[0]
        recvFrom = self.args[1]
        sendTo = self.args[2]

        for k, v in _clientDict.items():
            if (k == self.account):
                continue
            if (k != sendTo):
                continue
            sendToclientSocket(v.socket,
                               Request.sendMsg.value,
                               ResultCode.notify.value,
                               msg + ' [from: '+ recvFrom+']')


    def addFriend(self):
        print('加好友 参数: ', self.args)
        print('添加好友请求, self = ', self.account)

        # 添加好友
        # 客户端 -> 服务端, 参数: 好友的账号\n0|1|2
        #           0|1|2 : 0 表示这是一个添加好友的请求
        #                   1 表示这是一个同意被xxx添加为好友的回复(回复给服务端)
        #                   2 表示这是一个拒绝被xxx添加为好友的回复(回复给服务端)
        # 服务端 -> 客户端. 参数: 用户账号

        # 1. 判断是不是自己
        if self.account == self.args[0]:
            sendToclientSocket(self.socket,
                               Request.addFriend.value,
                               ResultCode.user_non_exist.value,
                               "不能添加自己")
            return
        # 1.1 判断要添加的用户账号是否存在
        sql = "select * from chatroom_user where account='%s';" % (self.args[0])
        _db.cursor.execute(sql)
        user = _db.cursor.fetchone()
        if user == None:
            sendToclientSocket(self.socket,
                               Request.addFriend.value,
                               ResultCode.user_non_exist.value,
                               "要添加的用户不存在")
            return

        # 2. 判断是否已经添加过了
        sql = "select * from chatroom_friend where user_1=%d and user_2=%d;" \
              % (self.id, user['id'])
        _db.cursor.execute(sql)
        # 获取查询的结果集,如果啥都没查到就返回None,否则就不会是None
        if _db.cursor.fetchone() != None:
            sendToclientSocket(self.socket,
                               Request.addFriend.value,
                               ResultCode.success.value,
                               "已添加过了")
            return

        # 判断是添加好友的请求还是回复. 0是请求, 1是回复(同意), 2是回复(拒绝)
        if self.args[1] == '0':
            # 3. 等待对方同意之后,再将两者的关系写入到好友关系表中.
            if user['account'] not in _clientDict:
                sendToclientSocket(self.socket,
                                   Request.addFriend.value,
                                   ResultCode.user_non_exist.value,
                                   "好友不在线")
                return
            # 将添加好友的申请发送给目标客户端
            sendToclientSocket(_clientDict[user['account']].socket,  # 目标的套接字
                               Request.addFriend.value,
                               ResultCode.notify.value,
                               self.account)

            # 将结果发回给发送了好友请求的客户端
            sendToclientSocket(self.socket,
                               Request.addFriend.value,
                               ResultCode.success.value,
                               "发送成功,等待对方接受")
            return
        elif self.args[1] == '1':  # 表示同意用户的添加好友请求, 此时可以将两者的关系写入到好友表
            # 1. 得到同意添加为好友对的用户的账号,根据账号查找数据库得到用户信息
            sql = "select * from chatroom_user where account='%s';" % (self.args[0])
            _db.cursor.execute(sql)
            user = _db.cursor.fetchone()
            # 2. 插入数据库
            sql = "insert into chatroom_friend (user_1, user_2) VALUES (%d,%d),(%d,%d);" % \
                  (self.id, user['id'], user['id'], self.id)
            _db.cursor.execute(sql)
            _db.cursor.execute("commit;")
            # 3. 可以通知对方,好友请求已经被同意了
        elif self.args[1] == '2':
            # 拒绝添加好友,可以通知对方,好友请求已经被拒绝了
            pass

    def getFriendList(self):
        print('获取好友列表 参数: ', self.args)
        # 查询数据库得到所有好友
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


def sendToclientSocket(clientSocket, type, status, strData):
    data = strData.encode('gb2312')
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
    print('type=%d , size=%d' % (type, size))

    # 接收后续的内容
    bodyData = clientSocket.recv(size)
    print('bodyData: %s' % (bodyData.decode('gb2312')))
    # 解码之后分割参数
    return type, bodyData.decode('gb2312').split('\n')


_clientDict = {}


class Server:
    def __init__(self, ip, port):
        # 1. 创建一个套接字
        self.socket = socket.socket()
        # 2. 绑定ip和端口
        self.socket.bind((ip, port))
        # 3. 监听
        self.socket.listen(socket.SOMAXCONN)

    def recvclientSocketData(self, handler):
        type, args = None, None

        while True:
            type, args = None, None
            try:
                # 接收数据
                type, args = recvFromclientSocket(handler.socket)
            except Exception as e:
                del _clientDict[handler.account]
                break
            try:
                # 将一个整型值转换成枚举类型
                # 如果这个整型值在枚举类型中没有定义
                # 就会抛出异常
                type = Request(type)
            except:
                sendToclientSocket(handler.socket,
                                   type,
                                   ResultCode.bad_pack.value,
                                   "无效的请求")
            # 1. 获取函数
            func = getattr(handler, type.name)
            # 2. 设置属性, 将客户端传来的参数设置成类对象的一个成员变量
            handler.args = args
            func()

    def run(self):
        # 4. 循环接收客户端连接
        while True:
            clientSocket, addr = self.socket.accept()
            h = Handler()
            h.socket = clientSocket
            h.addr = addr
            print('接入新客户端: ', addr)
            threading.Thread(
                target=self.recvclientSocketData,
                args=(h,)
            ).start()


if __name__ == '__main__':
    s = Server('127.0.0.1', 10086)
    s.run()
