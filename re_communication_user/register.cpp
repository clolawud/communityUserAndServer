#include "register.h"
#include "ui_register.h"

Register::Register(QWidget *parent,QTcpSocket *s)
    : QWidget(parent)
    , ui(new Ui::Register)
    , socket(s)
{
    ui->setupUi(this);
    connect(socket, &QTcpSocket::readyRead, this, &Register::responseSolt);
}

Register::~Register()
{
    delete ui;
}

void Register::on_backpushButton_clicked()
{
    this->close();
    emit backSignal();
}

void Register::on_registerpushButton_clicked()
{
    QJsonObject json;
    QJsonObject userJson;
    QString username = ui->namelineEdit->text();
    userJson["password"] = ui->passwordlineEdit->text();
    userJson["pending_requests"] = QJsonArray(); // 初始化好友申请列表
    userJson["friends"] = QJsonArray(); // 初始化好友列表
    json["cmd"] = "register";
    json[username] = userJson;
    QByteArray jsonData = QJsonDocument(json).toJson();
    socket->write(jsonData);
}

void Register::responseSolt(){
    QByteArray data = socket->readAll();
    QJsonDocument responseDoc = QJsonDocument::fromJson(data);
    QJsonObject response = responseDoc.object();
    if (response["cmd"].toString() == "register_response") {
        if (response["status"].toString() == "error") {
            QMessageBox::warning(this, "注册失败", "用户名已存在，请选择其他用户名。");
        } else if (response["status"].toString() == "success") {
            QMessageBox::information(this, "注册成功", "用户注册成功！现在可以登录了。");
            this->close();
            emit backSignal();
        }
    }
}