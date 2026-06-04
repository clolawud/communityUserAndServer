#ifndef LOG_H
#define LOG_H

#include <QWidget>
#include "register.h"
#include <QTcpSocket>
#include "use.h"

namespace Ui {
class Log;
}

class Log : public QWidget
{
    Q_OBJECT

public:
    explicit Log(QWidget *parent = nullptr,QTcpSocket *s = nullptr);
    ~Log();

private slots:
    void on_logpushButton_clicked();

    void on_registerpushButton_clicked();
    void responseSolt();

private:
    Ui::Log *ui;
    QTcpSocket *socket;
};

#endif // LOG_H
