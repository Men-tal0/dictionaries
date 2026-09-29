#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>
#include <QHostAddress>
#include <QString>
#include "msg.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_login_clicked();
    void onConnected();
    void onErrorOccurred(QAbstractSocket::SocketError);

    void on_delete_user_clicked();
    void onReadyRead();
    void on_connect_clicked();
    void on_return_2_clicked();

    void on_register_3_clicked();

    void on_search_clicked();

private:
    Ui::Widget *ui;
    QTcpSocket *tcpSocket;
};
#endif // WIDGET_H
