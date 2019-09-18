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
                                         password='niw123',
                                         db='chatroom',
                                         charset='utf8',
                                         cursorclass=pymysql.cursors.DictCursor)
        self.cursor = self.connecter.cursor(cursor=pymysql.cursors.DictCursor)

# 构造一个全局的数据库对象
_db = Database()

class Request(Enum):
    # 登陆请求
    # 客户端 -> 服务端, 参数: 账号\n密码
    login       = 1

    # 注册请求
    # 客户端 -> 服务端, 参数: 账号\n密码
    register    = 2

    # 发消息请求
    # 客户端 -> 服务端, 参数: 消息
    sendMultiMsg= 3




class ResultCode(Enum):
    notify    = -1
    success   = 0
    bad_pack  = 1
    user_exist = 2
    user_non_exist = 3
    password_wrong = 4

def md5( s ):
    hashObj = hashlib.md5()
    hashObj.update( s.encode('gb2312') )
    return hashObj.hexdigest()

class Handler():
    def __init__(self):
        self.args = None
        self.socket=None

    def register(self):
        print('注册 参数: ' , self.args)
        # 注册的流程
        # 1. 检查账号有没有注册过.
        sql = "select id from chatroom_user where account='%s';"%(self.args[0])
        # 执行sql语句
        _db.cursor.execute(sql)
        # 获取结果集
        result = _db.cursor.fetchone()
        if result != None:
            sendToclientSocket(self.socket,
                               Request.register.value,
                               ResultCode.user_exist.value,
                               "账号已存在")
            return
        # 2. 保存账号到数据库
        # passwd = md5(self.args[1])# here
        passwd = self.args[1]
        sql = "insert into chatroom_user (account, passwd) VALUE('%s','%s');" \
              %(self.args[0],passwd)
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')# 提交到数据库
        sendToclientSocket(self.socket,
                           Request.register.value,
                           ResultCode.success.value,
                           "注册成功")



    def login(self):
        print('登陆 参数: ' , self.args)
        # 1. 查表,判断用户是否存在
        sql = "select * from chatroom_user where account='%s';"%(self.args[0])
        _db.cursor.execute(sql)
        userInfo =_db.cursor.fetchone()
        if userInfo == None:
            sendToclientSocket(self.socket,
                               Request.login.value,
                               ResultCode.user_non_exist.value,
                               "账号不存在")
            return
        # 2. 比较密码是否正确
        if userInfo['passwd'] !=  self.args[1] :
        # if userInfo['passwd'] != md5(self.args[1]):
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
        msg = self.args[0]
        for k,v in _clientDict.items():
            if( k == self.account):
                continue
            sendToclientSocket( v.socket,
                                Request.sendMultiMsg.value,
                                ResultCode.notify.value,
                                msg)



def sendToclientSocket(clientSocket,type, status, strData):
    data = strData.encode('gb2312')
    size = len(data)
    strFormat = 'iii%ds'%(size)
    rawData = struct.pack( strFormat ,type, status, size,data)
    clientSocket.send( rawData )

def recvFromclientSocket(clientSocket):
    data = clientSocket.recv(8)
    if len(data) == 0:
        raise Exception('客户端断开连接')
    # 接收8个字节的头部.
    type, size = struct.unpack('ii', data)
    print('type=%d , size%d' % (type, size))

    # 接收后续的内容
    bodyData = clientSocket.recv(size)
    print('bodyData: %s' % (bodyData.decode('gb2312')))
    # 解码之后分割参数
    return type , bodyData.decode('gb2312').split('\n')


_clientDict = {}

class Server:
    def __init__(self, ip , port):
        # 1. 创建一个套接字
        self.socket = socket.socket()
        # 2. 绑定ip和端口
        self.socket.bind( (ip , port) )
        # 3. 监听
        self.socket.listen( socket.SOMAXCONN )

    def recvclientSocketData(self,handler):
        type,args = None,None

        while  True:
            type , args = None,None
            try:
                # 接收数据
                type , args = recvFromclientSocket(handler.socket)
            except Exception as e:
                del _clientDict[ handler.account ]
                break
            try:
                # 将一个整型值转换成枚举类型
                # 如果这个整型值在枚举类型中没有定义
                # 就会抛出异常
                type = Request(type)
            except:
                sendToclientSocket(handler.socket,
                                   type,
                                   ResultCode.bad_pack.value ,
                                   "无效的请求")
            # 1. 获取函数
            func = getattr(handler , type.name)
            # 2. 设置属性, 将客户端传来的参数设置成类对象的一个成员变量
            handler.args = args
            func()

    def run(self):
        # 4. 循环接收客户端连接
        while True:
            clientSocket , addr = self.socket.accept()
            h = Handler()
            h.socket = clientSocket
            h.addr = addr
            print('接入新客户端: ' , addr)
            threading.Thread(
                target=self.recvclientSocketData,
                args=(h,)
            ).start()


if __name__ == '__main__':
    s = Server('127.0.0.1',10086)
    s.run()
