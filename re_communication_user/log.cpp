#include "log.h"
#include "ui_log.h"

Log::Log(QWidget *parent, QTcpSocket *s)
    : QWidget(parent)
    , ui(new Ui::Log)
    , socket(s)  // 初始化 socket
{
    ui->setupUi(this);
    connect(socket, &QTcpSocket::readyRead, this, &Log::responseSolt);
}

Log::~Log()
{
    delete ui;
}

void Log::on_logpushButton_clicked()
{
    QJsonObject json;
    json["cmd"] = "login";
    json["logName"]=ui->namelineEdit->text();
    json["logPassWord"]=ui->passwordlineEdit->text();
    QByteArray jsonData = QJsonDocument(json).toJson();
    socket->write(jsonData);
}


void Log::on_registerpushButton_clicked()
{
    this->hide();
    disconnect(socket, &QTcpSocket::readyRead, this, &Log::responseSolt);
    Register *r;
    r = new Register(nullptr,socket);
    r->show();
    connect(r, &Register::backSignal, this, [=](){
    disconnect(socket, &QTcpSocket::readyRead, r, &Register::responseSolt);
    connect(socket, &QTcpSocket::readyRead, this, &Log::responseSolt);
    r->deleteLater();
    this->show();
    });
}
void Log::responseSolt(){
    QByteArray data = socket->readAll();
    QJsonDocument responseDoc = QJsonDocument::fromJson(data);
    QJsonObject response = responseDoc.object();
    if (response["cmd"].toString() == "login_response") {
        if (response["logStatus"].toString() == "error") {
            QMessageBox::warning(this, "登录失败", "用户名或密码错误，请重新输入。");
        } else if (response["logStatus"].toString() == "success") { 
            QMessageBox::information(this, "登录成功", "用户登录成功！");
            QString username = response["newUser"].toString();
            // 在这里可以添加登录成功后的操作，例如打开主界面等
            this->hide();
            disconnect(socket, &QTcpSocket::readyRead, this, &Log::responseSolt);
            Use *u = new Use(username,nullptr, socket);
            u->show();
            connect(u, &Use::outLogSlot, this, [=](){
            disconnect(socket, &QTcpSocket::readyRead, u, &Use::useResponseSolt);
            connect(socket, &QTcpSocket::readyRead, this, &Log::responseSolt);
            u->deleteLater();
            this->show();
            });
            qDebug() << "username 的值是:" << username;  // 添加这行
        }
    }
}
