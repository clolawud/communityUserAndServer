#include "widget.h"
#include "./ui_widget.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget){
    ui->setupUi(this);
    server = new QTcpServer(this);
    server->listen(QHostAddress::AnyIPv4, 8000);
    connect(server, &QTcpServer::newConnection, this, &Widget::NewConnectionSlot);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("chat.db");
    if (!db.open()) {
        qDebug() << "数据库打开失败:" << db.lastError().text();
        return;
    }
    QSqlQuery query(db);
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "username TEXT PRIMARY KEY, "
               "password TEXT NOT NULL)");
    query.exec("CREATE TABLE IF NOT EXISTS friends ("
               "username TEXT, "
               "friendname TEXT, "
               "PRIMARY KEY (username, friendname))");
    query.exec("CREATE TABLE IF NOT EXISTS pending_requests ("
               "from_user TEXT, "
               "to_user TEXT, "
               "PRIMARY KEY (from_user, to_user))");
    query.exec("CREATE TABLE IF NOT EXISTS messages ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "from_user TEXT, "
               "to_user TEXT, "
               "content TEXT)");
}

Widget::~Widget(){
    delete ui;
    delete server;
}

void Widget::NewConnectionSlot(){
    QTcpSocket *newSocket = server->nextPendingConnection();
    ui->IPlineEdit->setText(newSocket->peerAddress().toString());
    ui->portlineEdit->setText(QString::number(newSocket->peerPort()));

    // 每个连接独立绑定
    connect(newSocket, &QTcpSocket::readyRead, this, [this, newSocket](){
        QByteArray userSignal = newSocket->readAll();
        QJsonDocument userDoc = QJsonDocument::fromJson(userSignal);
        QJsonObject userObj = userDoc.object();
        if(userObj["cmd"].toString() == "register")
            registerSlot(userObj, newSocket);//传递客户端信息和socket
        if(userObj["cmd"].toString() == "login")
            loginSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "add_friend")
            addFriendSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "get_pending_requests")
            pendFriendSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "accept_friend")
            acceptFriendSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "get_friends")
            getFriendsSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "delete_friend")
            deleteFriendSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "send_message")
            sendMessageSlot(userObj, newSocket);
        if(userObj["cmd"].toString() == "get_messages")
            getMessagesSlot(userObj, newSocket);
    });

    // 断开时清理
    connect(newSocket, &QTcpSocket::disconnected, this, [this, newSocket](){
        clients.remove(newSocket);
        newSocket->deleteLater();
    });
}

// ============== 注册 ==============
void Widget::registerSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username;
    QJsonObject userObj;
    for (auto it = request.begin(); it != request.end(); ++it) {
        if (it.key() != "cmd") {
            username = it.key();
            userObj = it.value().toObject();
            break;
        }
    }
    QString password = userObj["password"].toString();

    QSqlQuery query;
    QJsonObject response;

    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);
    query.exec();
    query.next();
    if (query.value(0).toInt() > 0) {
        qDebug() << "用户已存在";
        response["cmd"] = "register_response";
        response["status"] = "error";
        sock->write(QJsonDocument(response).toJson());
        return;
    }

    query.prepare("INSERT INTO users (username, password) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(password);
    query.exec();

    qDebug() << "用户创建成功";
    response["cmd"] = "register_response";
    response["status"] = "success";
    sock->write(QJsonDocument(response).toJson());
}

// ============== 登录 ==============
void Widget::loginSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["logName"].toString();
    QString password = request["logPassWord"].toString();

    QSqlQuery query;
    query.prepare("SELECT password FROM users WHERE username = ?");
    query.addBindValue(username);
    query.exec();

    QJsonObject response;
    if (query.next() && query.value(0).toString() == password) {
        qDebug() << "登录成功:" << username;
        clients[sock] = username;   // 登记：这个 socket 属于谁
        response["cmd"] = "login_response";
        response["logStatus"] = "success";
        response["newUser"] = username;
    } else {
        qDebug() << "登录失败";
        response["cmd"] = "login_response";
        response["logStatus"] = "error";
    }
    sock->write(QJsonDocument(response).toJson());
}

// ============== 添加好友 ==============
void Widget::addFriendSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["username"].toString();
    QString friendname = request["friendname"].toString();
    QSqlQuery query;
    QJsonObject response;

    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(friendname);
    query.exec();
    query.next();
    if (query.value(0).toInt() == 0) {
        qDebug() << "好友不存在";
        response["cmd"] = "add_friend_response";
        response["status"] = "error";
        response["message"] = "好友不存在";
        sock->write(QJsonDocument(response).toJson());
        return;
    }

    query.prepare("SELECT COUNT(*) FROM friends WHERE username = ? AND friendname = ?");
    query.addBindValue(username);
    query.addBindValue(friendname);
    query.exec();
    query.next();
    if (query.value(0).toInt() > 0) {
        qDebug() << "已经是好友了";
        response["cmd"] = "add_friend_response";
        response["status"] = "error";
        response["message"] = "已经是好友了";
        sock->write(QJsonDocument(response).toJson());
        return;
    }

    query.prepare("INSERT OR IGNORE INTO pending_requests (from_user, to_user) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(friendname);
    query.exec();

    qDebug() << "好友申请已发送";
    response["cmd"] = "add_friend_response";
    response["status"] = "success";
    response["message"] = "好友申请已发送";
    sock->write(QJsonDocument(response).toJson());
}

// ============== 查看好友申请列表 ==============
void Widget::pendFriendSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["username"].toString();
    QSqlQuery query;
    QJsonObject response;
    QJsonArray requests;

    query.prepare("SELECT from_user FROM pending_requests WHERE to_user = ?");
    query.addBindValue(username);
    query.exec();
    while (query.next()) {
        requests.append(query.value(0).toString());
    }

    response["cmd"] = "pending_requests_response";
    response["requests"] = requests;
    sock->write(QJsonDocument(response).toJson());
}

// ============== 接受好友申请 ==============
void Widget::acceptFriendSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["username"].toString();
    QString friendname = request["friendname"].toString();
    QSqlQuery query;
    QJsonObject response;

    query.prepare("SELECT COUNT(*) FROM pending_requests "
                  "WHERE from_user = ? AND to_user = ?");
    query.addBindValue(friendname);
    query.addBindValue(username);
    query.exec();
    query.next();
    if (query.value(0).toInt() == 0) {
        response["cmd"] = "accept_friend_response";
        response["status"] = "error";
        response["message"] = "没有该好友申请";
        sock->write(QJsonDocument(response).toJson());
        return;
    }

    query.prepare("DELETE FROM pending_requests WHERE from_user = ? AND to_user = ?");
    query.addBindValue(friendname);
    query.addBindValue(username);
    query.exec();

    query.prepare("INSERT OR IGNORE INTO friends (username, friendname) VALUES (?, ?)");
    query.addBindValue(username);
    query.addBindValue(friendname);
    query.exec();

    query.prepare("INSERT OR IGNORE INTO friends (username, friendname) VALUES (?, ?)");
    query.addBindValue(friendname);
    query.addBindValue(username);
    query.exec();

    qDebug() << username << "和" << friendname << "已成为好友";
    response["cmd"] = "accept_friend_response";
    response["status"] = "success";
    response["message"] = "好友添加成功";
    response["friendname"] = friendname;
    sock->write(QJsonDocument(response).toJson());
}

// ============== 获取好友列表 ==============
void Widget::getFriendsSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["username"].toString();
    QSqlQuery query;
    QJsonObject response;
    QJsonArray friends;

    query.prepare("SELECT friendname FROM friends WHERE username = ?");
    query.addBindValue(username);
    query.exec();
    while (query.next()) {
        friends.append(query.value(0).toString());
    }

    response["cmd"] = "get_friends_response";
    response["friends"] = friends;
    sock->write(QJsonDocument(response).toJson());
}

// ============== 删除好友 ==============
void Widget::deleteFriendSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["username"].toString();
    QString friendname = request["friendname"].toString();
    QSqlQuery query;
    QJsonObject response;

    query.prepare("SELECT COUNT(*) FROM friends WHERE username = ? AND friendname = ?");
    query.addBindValue(username);
    query.addBindValue(friendname);
    query.exec();
    query.next();
    if (query.value(0).toInt() == 0) {
        response["cmd"] = "delete_friend_response";
        response["status"] = "error";
        response["message"] = "该用户不是您的好友";
        sock->write(QJsonDocument(response).toJson());
        return;
    }

    query.prepare("DELETE FROM friends WHERE username = ? AND friendname = ?");
    query.addBindValue(username);
    query.addBindValue(friendname);
    query.exec();

    query.prepare("DELETE FROM friends WHERE username = ? AND friendname = ?");
    query.addBindValue(friendname);
    query.addBindValue(username);
    query.exec();

    qDebug() << username << "已删除好友" << friendname;
    response["cmd"] = "delete_friend_response";
    response["status"] = "success";
    response["friendname"] = friendname;
    response["message"] = "好友删除成功";
    sock->write(QJsonDocument(response).toJson());
}

// ============== 发送消息 ==============
void Widget::sendMessageSlot(const QJsonObject &request, QTcpSocket *sock){
    QString from = request["from"].toString();
    QString to = request["to"].toString();
    QString content = request["content"].toString();
    QSqlQuery query;
    QJsonObject response;

    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(from);
    query.exec();
    query.next();
    bool fromExists = query.value(0).toInt() > 0;

    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(to);
    query.exec();
    query.next();
    bool toExists = query.value(0).toInt() > 0;

    if (!fromExists || !toExists) {
        response["cmd"] = "send_message_response";
        response["status"] = "error";
        response["message"] = "用户不存在";
        sock->write(QJsonDocument(response).toJson());
        return;
    }

    // 插入数据库
    query.prepare("INSERT INTO messages (from_user, to_user, content) VALUES (?, ?, ?)");
    query.addBindValue(from);
    query.addBindValue(to);
    query.addBindValue(content);
    query.exec();

    // 回复发送者：成功
    response["cmd"] = "send_message_response";
    response["status"] = "success";
    sock->write(QJsonDocument(response).toJson());

    // 如果对方在线，实时推送
    for (auto it = clients.begin(); it != clients.end(); ++it) {
        if (it.value() == to) {
            QJsonObject push;
            push["cmd"] = "new_message";
            push["from"] = from;
            push["content"] = content;
            it.key()->write(QJsonDocument(push).toJson());
            break;
        }
    }
}

// ============== 获取聊天记录 ==============
void Widget::getMessagesSlot(const QJsonObject &request, QTcpSocket *sock){
    QString username = request["username"].toString();
    QString friendname = request["friendname"].toString();
    QSqlQuery query;
    QJsonObject response;
    QJsonArray messages;

    query.prepare("SELECT from_user, content FROM messages "
                  "WHERE (from_user = ? AND to_user = ?) "
                  "   OR (from_user = ? AND to_user = ?) "
                  "ORDER BY id ASC");
    query.addBindValue(username);
    query.addBindValue(friendname);
    query.addBindValue(friendname);
    query.addBindValue(username);
    query.exec();

    while (query.next()) {
        QJsonObject msg;
        msg["from"] = query.value(0).toString();
        msg["content"] = query.value(1).toString();
        messages.append(msg);
    }

    response["cmd"] = "get_messages_response";
    response["messages"] = messages;
    sock->write(QJsonDocument(response).toJson());
}
