#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    socket = new QTcpSocket(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_connectpushButton_clicked()
{
    socket->connectToHost(ui->IPlineEdit->text(), ui->portlineEdit->text().toInt());
    if (!socket->waitForConnected(3000)) {
        // 连接失败，停留在当前界面
        return;
    }
    Log *L = new Log(nullptr, socket);
    L->show();
    this->hide();
}


void Widget::on_closepushButton_clicked()
{
    this->close();
}

