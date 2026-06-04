#ifndef USE_H
#define USE_H

#include <QWidget>
#include <QTcpSocket>
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

namespace Ui {
class Use;
}

class Use : public QWidget
{
    Q_OBJECT

public:
    explicit Use(QString &u,QWidget *parent = nullptr,QTcpSocket *s = nullptr);
    QString username;
    QTcpSocket *socket;  // 添加 socket 成员变量
    ~Use();

public slots:
    void on_logOutpushButton_clicked();

    void on_addFriendpushButton_clicked();

    void on_friendApplyPushButton_clicked();
    void responseSolt(const QJsonObject &response);
    void addFesponseSolt(const QJsonObject &response);
    void acceptFesponseSolt(const QJsonObject &response);
    void friendListSolt(const QJsonObject &response);
    void useResponseSolt();
    void deleteFriendSolt(const QJsonObject &response);
    void sendMessageSolt(const QJsonObject &response);
    void getMessagesSolt(const QJsonObject &response);
    void on_sendMessagePushButton_clicked();
private:signals:
    void outLogSlot();

private slots:
    void on_deleteFriendPushButton_clicked();

private:
    Ui::Use *ui;
};

#endif // USE_H
