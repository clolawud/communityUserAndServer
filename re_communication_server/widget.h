#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostAddress>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QJsonArray>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;
public slots:
    void NewConnectionSlot();
private:
    void registerSlot(const QJsonObject &request, QTcpSocket *sock);
    void loginSlot(const QJsonObject &request, QTcpSocket *sock);
    void addFriendSlot(const QJsonObject &request, QTcpSocket *sock);
    void pendFriendSlot(const QJsonObject &request, QTcpSocket *sock);
    void acceptFriendSlot(const QJsonObject &request, QTcpSocket *sock);
    void getFriendsSlot(const QJsonObject &request, QTcpSocket *sock);
    void deleteFriendSlot(const QJsonObject &request, QTcpSocket *sock);
    void sendMessageSlot(const QJsonObject &request, QTcpSocket *sock);
    void getMessagesSlot(const QJsonObject &request, QTcpSocket *sock);

public:signals:
    void existSignal();
    void successSignal();

private:
    Ui::Widget *ui;
    QTcpServer *server;
    QMap<QTcpSocket*, QString> clients;   // socket → username
};
#endif // WIDGET_H
