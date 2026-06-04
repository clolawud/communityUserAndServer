#include "use.h"
#include "ui_use.h"
#include <QListWidget>

Use::Use(QString &u, QWidget *parent, QTcpSocket *s)
    : QWidget(parent)
    , ui(new Ui::Use)
    , socket(s)
    ,username(u){
    ui->setupUi(this);
    ui->useNameLabel->setText(username);
    connect(socket, &QTcpSocket::readyRead, this, &Use::useResponseSolt);
    connect(ui->friendListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString friendName = item->text();
        ui->friendNameLabel->setText(friendName);
        ui->friendStateLabel->setText("在线");
        // 点击好友时拉取聊天记录
        QJsonObject msgJson;
        msgJson["cmd"] = "get_messages";
        msgJson["username"] = username;
        msgJson["friendname"] = friendName;
        socket->write(QJsonDocument(msgJson).toJson());
    });
    QJsonObject json;
    json["cmd"] = "get_friends";
    json["username"] = username;
    socket->write(QJsonDocument(json).toJson());
}

Use::~Use(){
    delete ui;
}
//服务器返回的响应处理
void Use::useResponseSolt(){
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject response = doc.object();
    if (response["cmd"] == "add_friend_response"){
        addFesponseSolt(response);//添加好友请求响应
    }
    if (response["cmd"] == "pending_requests_response"){
        responseSolt(response);//
    }
    if (response["cmd"] == "accept_friend_response"){
        acceptFesponseSolt(response);
    }
    if (response["cmd"] == "get_friends_response"){
        friendListSolt(response);
    }
    if (response["cmd"] == "delete_friend_response")
        deleteFriendSolt(response);
    if (response["cmd"] == "send_message_response")
        sendMessageSolt(response);
    if (response["cmd"] == "get_messages_response")
        getMessagesSolt(response);
    if (response["cmd"] == "new_message") {
        // 收到实时推送，重新拉取当前聊天记录
        QListWidgetItem *cur = ui->friendListWidget->currentItem();
        if (cur) {
            QJsonObject json;
            json["cmd"] = "get_messages";
            json["username"] = username;
            json["friendname"] = cur->text();
            socket->write(QJsonDocument(json).toJson());
        }
    }
}
//登出按钮
void Use::on_logOutpushButton_clicked(){
    this->close();
    emit outLogSlot();
}



//添加好友按钮，发送添加谁给服务器，服务器根据情况返回不同的响应
void Use::on_addFriendpushButton_clicked(){
    bool ok;        //使用了输入对话框，获取好友ID，对话款的只用就是获取文本
    QString friendName = QInputDialog::getText(this,"添加好友","请输入好友ID",QLineEdit::Normal,"",&ok);
    if (ok && !friendName.isEmpty()) {
        if(friendName == username) {
            QMessageBox::warning(this,"错误","不能添加自己为好友");
            return;
        }
        QJsonObject json;
        json["cmd"] = "add_friend";
        json["username"] = username;
        json["friendname"] = friendName;
        QByteArray data = QJsonDocument(json).toJson();
        socket->write(data);
    }
}
//接收服务器好友申请响应处理后的不同结果
void Use::addFesponseSolt(const QJsonObject &response){
    QString message = response["message"].toString();
    if(response["message"] == "好友不存在"){
        QMessageBox::warning(this,"警告","好友不存在");
    }
    if(response["message"] == "已经是好友了"){
        QMessageBox::warning(this,"警告","已经是好友了");
    }
    if(response["message"] == "好友申请已发送"){
        QMessageBox::information(this,"提示","申请已发送");
    }
}




//好友申请按钮，向服务器发送请求查看好友列表请求
void Use::on_friendApplyPushButton_clicked(){
    QJsonObject json;
    json["cmd"] = "get_pending_requests";
    json["username"] = username;  // 告诉服务器你是谁
    socket->write(QJsonDocument(json).toJson());
}
//接收服务器发送的好友申请列表就行显示，同时选择是否添加
void Use::responseSolt(const QJsonObject &response){
    QJsonArray requests = response["requests"].toArray();
    if (requests.isEmpty()) {
        QMessageBox::information(this, "好友申请", "没有新的好友申请");
    } else {
        QStringList requestList;
        for (const QJsonValue &value : requests) {
            requestList.append(value.toString());
        }
        bool ok;
        QString selectedFriend = QInputDialog::getItem(this, "好友申请", "选择要接受的好友申请:", requestList, 0, false, &ok);
        if (ok && !selectedFriend.isEmpty()) {
            // 这里可以发送一个接受好友的请求给服务器
            QJsonObject json;
            json["cmd"] = "accept_friend";
            json["username"] = username;
            json["friendname"] = selectedFriend;
            socket->write(QJsonDocument(json).toJson());
        }
    }
}
//在服务器处理完接受好友请求后返回结果，客户端根据结果进行不同的提示
void Use::acceptFesponseSolt(const QJsonObject &response){
    if (response["status"].toString() == "success") {
        QString friendname = response["friendname"].toString();
        QMessageBox::information(this, "好友添加成功",
            friendname + " 已成为您的好友！");
        QJsonObject json;
        json["cmd"] = "get_friends";
        json["username"] = username;
        socket->write(QJsonDocument(json).toJson());
    } else {
        QString message = response["message"].toString();
        QMessageBox::warning(this, "好友添加失败", message);
    }
}






//删除好友按钮，向服务器发送删除所选的好友请求
void Use::on_deleteFriendPushButton_clicked(){
    QListWidgetItem *currentItem = ui->friendListWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "删除好友", "请先在好友列表中选择一个好友");
        return;
    }
    QString deleteFriendName = currentItem->text();
    if(deleteFriendName == username) {
        QMessageBox::warning(this,"错误","不能删除自己");
        return;
    }
    QMessageBox::StandardButton reply = QMessageBox::question(
    this, "删除好友",
    "确定要删除好友 " + deleteFriendName + " 吗？",
    QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;
    QJsonObject json;
    json["cmd"] = "delete_friend";
    json["username"] = username;
    json["friendname"] = deleteFriendName;
    QByteArray data = QJsonDocument(json).toJson();
    socket->write(data);
}
//接收服务器删除好友处理结果，显示结果，并刷新显示好友列表
void Use::deleteFriendSolt(const QJsonObject &response){
    if (response["status"].toString() == "success") {
        QString friendname = response["friendname"].toString();
        QMessageBox::information(this, "好友删除成功",
            friendname + " 已从您的好友列表中删除！");
        // 删除成功后刷新好友列表
        QJsonObject json;
        json["cmd"] = "get_friends";
        json["username"] = username;
        socket->write(QJsonDocument(json).toJson());
    } else {
        QString message = response["message"].toString();
        QMessageBox::warning(this, "好友删除失败", message);
    }
}
//显示好友列表，在ui建立时，添加好友时，删除好友时，调用向服务器发送显示好友列表请求
void Use::friendListSolt(const QJsonObject &response){
    ui->friendListWidget->clear();
    QJsonArray friends = response["friends"].toArray();
    for (const QJsonValue &val : friends) {
        ui->friendListWidget->addItem(val.toString());
    }
}




//发送消息按钮
void Use::on_sendMessagePushButton_clicked(){
    QListWidgetItem *currentItem = ui->friendListWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "发送失败", "请先在好友列表中选择一个好友");
        return;
    }
    QString friendName = currentItem->text();
    QString content = ui->inMessageTextBrowser->toPlainText().trimmed();
    if (content.isEmpty()) {
        QMessageBox::warning(this, "发送失败", "消息内容不能为空");
        return;
    }
    QJsonObject json;
    json["cmd"] = "send_message";
    json["from"] = username;
    json["to"] = friendName;
    json["content"] = content;
    socket->write(QJsonDocument(json).toJson());
}
//发送消息响应处理
void Use::sendMessageSolt(const QJsonObject &response){
    if (response["status"].toString() == "success") {
        // 发送成功后，重新拉取聊天记录
        QListWidgetItem *currentItem = ui->friendListWidget->currentItem();
        if (currentItem) {
            QJsonObject json;
            json["cmd"] = "get_messages";
            json["username"] = username;
            json["friendname"] = currentItem->text();
            socket->write(QJsonDocument(json).toJson());
        }
        ui->inMessageTextBrowser->clear();
    } else {
        QMessageBox::warning(this, "发送失败", response["message"].toString());
    }
}
//获取消息响应处理
void Use::getMessagesSolt(const QJsonObject &response){
    ui->MessageTextBrowser->clear();
    QJsonArray messages = response["messages"].toArray();
    for (const QJsonValue &val : messages) {
        QJsonObject msg = val.toObject();
        QString from = msg["from"].toString();
        QString content = msg["content"].toString();
        ui->MessageTextBrowser->append(from + ": " + content);
    }
}