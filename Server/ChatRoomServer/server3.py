# V1
#   1. 建立能够接收客户端连接的服务端
#   2. 收发客户端的数据
#   2.1 制定一个客户端和服务端数据传输的标准
# V2
#   3. 实现服务端群发客户端消息
#       3.1 其中一个客户端发送消息, 其他在线的客户端都能接收到该消息.
#   4. 实现服务端给客户端转发消息
#       4.1 客户端上线时必须起一个名字. 不起名字就没有这个功能
#       4.2 客户端在发送消息时, 消息如果以 `@xxx 消息内容` 的格式发送.
# #           服务端就只将消息的内容发给具有指定名字的客户端
# V3
#   5. 实现注册,登陆功能
#   5.1 数据库用户表的设计
#   5.2 插入语句
#   5.3 查询语句
# V4
#   6. 实现好友功能
# V5
#   7. 实现群功能

import hashlib
import random
import socket
import struct
import threading
from enum import Enum
import base64

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
    # 客户端 -> 服务端, 参数: 群名称\n消息
    # 服务度 -> 客户端, 参数: 群名称\n发送者账号\n消息内容
    sendMultiMsg= 3

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

    # 创建群
    # 客户端 -> 服务端, 参数: 群的名称
    createroom = 7

    # 加入群
    # 客户端 -> 服务端, 参数: 群的名称
    joincroom = 8

    # 获取所加入的所有群的列表
    # 客户端 -> 服务端, 参数: 无
    # 服务端 -> 客户端, 返回内容: 群1名称\n群2名称\n群n名称
    getroomlist = 9

    # 获取群内成员列表
    # 客户端 -> 服务端, 参数: 群的名称
    # 服务端 -> 客户端, 返回内容: 用户1名称\n用户2名称\n用户n名称
    getroommember = 10

class ResultCode(Enum):
    notify    = -1  	# 表示消息是由服务端主动发起的通知
    success   = 0		# 处理成功
    bad_pack  = 1		# 不正确的数据包
    user_exist = 2		# 用户已存在
    user_non_exist = 3	# 用户不存在
    password_wrong = 4	# 密码错误
    room_exist = 5  # 群已存在
    already_in_room = 6# 已经在群里了


def md5( s ):
    """
	计算字符串的MD5
	"""
    hashObj = hashlib.md5()
    hashObj.update( s.encode('gb2312') )
    return hashObj.hexdigest()

class Handler():
    """
	客户端请求的处理类
	"""

    def __init__(self):
        self.args = None
        self.socket=None
        self.account = ''
        self.id = 0

    def register(self):
        """
		注册
		"""
        print('注册 参数: ' , self.args)
        # 注册的流程
        # 1. 检查账号有没有注册过.
        sql = "select * from chatroom_user where account='%s';"%(self.args[0])
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
        # passwd = md5(self.args[1])
        passwd = self.args[1]# here
        sql = "insert into chatroom_user (account, passwd) VALUE('%s','%s');" \
              %(self.args[0],passwd)
        # 执行sql语句
        _db.cursor.execute(sql)
        # 提交sql语句的更改
        _db.cursor.execute('commit;')# 提交到数据库

        # 将结果发送给客户端
        sendToclientSocket(self.socket,
                           Request.register.value,
                           ResultCode.success.value,
                           "注册成功")



    def login(self):
        print('登陆 参数: ' , self.args)
        # 1. 查表,判断用户是否存在
        sql = "select * from chatroom_user where account='%s';"%(self.args[0])
        _db.cursor.execute(sql)
        # 获取sql语句执行后的结果集.
        # 结果集是一个字典,键就是表的字段名, 值就是表中的值.
        userInfo =_db.cursor.fetchone()

        # 如果没有结果集, 函数会返回None
        if userInfo == None:
            # 如果在数据库找不到用户的信息,则说明用户未注册
            # 将结果发回给客户端
            sendToclientSocket(self.socket,
                               Request.login.value,
                               ResultCode.user_non_exist.value,
                               "用户名不存在")
            return

        # 2. 比较密码是否正确
        if userInfo['passwd'] != self.args[1]:  # here
        #if userInfo['passwd'] != md5( self.args[1] ):
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
        # roomName,msg = self.args[0],self.args[1]

        msg = self.args[0]
        recvFrom = self.args[1]
        sendTo = self.args[2]
        # 1. 找到群内的所有成员(除自己以外)
        sql = "select * from chatroom_room where account='%s';" % (sendTo)
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if not room:
            sendToclientSocket(self.socket,
                               Request.getroommember.value,
                               ResultCode.room_exist.value,
                               "此群不存在")
            return
        sql = "select * from chatroom_user where id in(" \
              "select id_user from chatroom_roommember where id_room=%d);" % (room['id'])
        _db.cursor.execute(sql)
        memberAll = _db.cursor.fetchall()
        # 2. 遍历成员发送消息
        #msg = '%s\n%s\n%s'%(sendTo,self.account,msg)
        for u in memberAll:
            if u['account'] in _clientDict:# 判断群用户是否在线
                sendToclientSocket(_clientDict[ u['account'] ].socket,
                                   Request.sendMultiMsg.value,
                                   ResultCode.notify.value,
                                   msg + ' [from: '+sendTo+'群-'+ recvFrom+']')

    def sendMsg(self):# here
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
        sql="select * from chatroom_user where account='%s';"%(self.args[0])
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
              %(self.id , user['id'])
        _db.cursor.execute( sql )
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
        elif self.args[1] == '1': # 表示同意用户的添加好友请求, 此时可以将两者的关系写入到好友表
            # 1. 得到同意添加为好友对的用户的账号,根据账号查找数据库得到用户信息
            sql = "select * from chatroom_user where account='%s';" % (self.args[0])
            _db.cursor.execute(sql)
            user = _db.cursor.fetchone()
            # 2. 插入数据库
            sql = "insert into chatroom_friend (user_1, user_2) VALUES (%d,%d),(%d,%d);"% \
                  (self.id,user['id'],user['id'],self.id)
            _db.cursor.execute(sql)
            _db.cursor.execute("commit;")
            # 3. 可以通知对方,好友请求已经被同意了

        elif self.args[1] == '2':
            # 拒绝添加好友
            # 可以通知对方,好友请求已经被拒绝了
            pass


    def getFriendList(self):
        # 查询数据库得到所有好友
        print('获取好友列表 参数: ', self.args)
        sql = "select * from chatroom_user where id in " \
              "(select user_2 from chatroom_friend where user_1 = %d);"%(self.id)
        _db.cursor.execute(sql)
        # 获取结果集
        all = _db.cursor.fetchall()
        friendList = []
        # 遍历结果集
        for f in all:
            friendList.append( f['account'])
        # 将所有好友的名字使用\n作为分隔符合并成字符串
        fList =  '\n'.join( friendList )
        # 发送给客户端.
        sendToclientSocket(self.socket,
                           Request.getFriendList.value,
                           ResultCode.success.value,
                           fList)
    def createroom(self):
        print('创建群 参数: ', self.args)
        # 创建群
        # 客户端 -> 服务端, 参数: 群的名称
        roomName = self.args[0]
        # 1. 检查群是否已经创建
        sql = "select * from chatroom_room where account='%s';"%(roomName)
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if room :# 群存在了
            sendToclientSocket(self.socket,
                               Request.createroom.value,
                               ResultCode.room_exist.value,
                               "群已经存在")
            return
        # 2. 创建一个新的
        sql = "insert into chatroom_room (account) VALUES('%s');"%(roomName)
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')
        # 3. 将自己自动加入到新群中,并将身份设置为群主
        # 3.1 获取群在数据表中的id
        sql = "select * from chatroom_room where account='%s';" % (roomName)
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        sql = "insert into chatroom_roommember (id_user, id_room,user_status) " \
              " VALUES(%d,%d,0)"%(self.id , room['id'])
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')
        # 4. 返回成功
        sendToclientSocket(self.socket,
                           Request.createroom.value,
                           ResultCode.success.value,
                           "可不能为空啊，为空则玄学bug")

    # 加入群
    # 客户端 -> 服务端, 参数: 群的名称
    def joincroom(self):
        print('加入群 参数: ', self.args)
        roomName = self.args[0]
        # 1. 判断群是否已存在
        sql = "select * from chatroom_room where account='%s';" % (roomName)
        _db.cursor.execute(sql)
        room = _db.cursor.fetchone()
        if not room:  # 群存在了
            sendToclientSocket(self.socket,
                               Request.joincroom.value,
                               ResultCode.room_exist.value,
                               "不存在这个群")
            return
        # 2. 是否已经加入过
        sql ="select * from chatroom_roommember where " \
             "id_room=%d and id_user=%d;"%(room['id'],self.id)
        _db.cursor.execute(sql)
        if _db.cursor.fetchone() :
            sendToclientSocket(self.socket,
                               Request.joincroom.value,
                               ResultCode.already_in_room.value,# here
                               "早就在群里了")
            return
        # 3. 可以找到群的创建者,将消息发送创建者让他决定是否允许别人加入到这个群
        # 4. 这里直接无条件加入
        sql = "insert into chatroom_roommember (id_user, id_room, user_status) " \
              "VALUES (%d,%d,2)"%(self.id,room['id'])
        _db.cursor.execute(sql)
        _db.cursor.execute('commit;')
        sendToclientSocket(self.socket,
                           Request.joincroom.value,
                           ResultCode.success.value,
                           "可不能为空啊，为空则玄学bug")

    # 获取所加入的所有群的列表
    # 客户端 -> 服务端, 参数: 无
    # 服务端 -> 客户端, 返回内容: 群1名称\n群2名称\n群n名称
    def getroomlist(self):
        print('获取群列表 参数: ', self.args)
        sql = "select * from chatroom_room where id in( " \
              " select id_room from chatroom_roommember where id_user=%d);"%(self.id)
        _db.cursor.execute(sql)
        all = _db.cursor.fetchall()
        roomNameList = []
        for r in all:
            roomNameList.append(r['account'])

        names =  '\n'.join(roomNameList)
        sendToclientSocket(self.socket,
                           Request.getroomlist.value,
                           ResultCode.success.value,
                           names)

    # 获取群内成员列表
    # 客户端 -> 服务端, 参数: 群的名称
    # 服务端 -> 客户端, 返回内容: 用户1名称\n用户2名称\n用户n名称
    def getroommember(self):
        print('获取群成员 参数: ', self.args)
        roomName = self.args[0]
        sql = "select * from chatroom_room where account='%s';"%(roomName)
        _db.cursor.execute(sql)
        room= _db.cursor.fetchone()
        if not room:
            sendToclientSocket(self.socket,
                               Request.getroommember.value,
                               ResultCode.room_exist.value,
                               "此群不存在")
            return
        sql = "select * from chatroom_user where id in(" \
              "select id_user from chatroom_roommember where id_room=%d);"%(room['id'])
        _db.cursor.execute(sql)
        all = _db.cursor.fetchall()
        friendList=[]
        for f in all:
            friendList.append(f['account'])
        names = '\n'.join(friendList)
        sendToclientSocket(self.socket,
                           Request.getroommember.value,
                           ResultCode.success.value,
                           names)


def sendToclientSocket(clientSocket,type, status, strData):
    data = strData.encode('gb2312')
    # 发之间先加密（加密盒子里的数据，而不是盒子本身）
    data = base64.b64encode(data)#here

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
    # 接收后先解密
    bodyData = base64.b64decode(bodyData)# here

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