#ifndef REGISTER_H
#define REGISTER_H

#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonArray>


namespace Ui {
class Register;
}

class Register : public QWidget
{
    Q_OBJECT

public:
    explicit Register(QWidget *parent = nullptr, QTcpSocket *s = nullptr);
    ~Register();

public slots:
    void on_backpushButton_clicked();

    void on_registerpushButton_clicked();
    void responseSolt();

private:
    Ui::Register *ui;
    QTcpSocket *socket;
public:signals:
    void backSignal();
};

#endif // REGISTER_H
