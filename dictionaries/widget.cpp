#include "widget.h"
#include "ui_widget.h"


Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    tcpSocket = new QTcpSocket(this);


    connect(tcpSocket,&QTcpSocket::connected,this,&Widget::onConnected);
    connect(tcpSocket,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(onErrorOccurred(QAbstractSocket::SocketError)));
    connect(tcpSocket, &QTcpSocket::readyRead, this, &Widget::onReadyRead);
    connect(ui->register_2,&QPushButton::clicked,this,[this]()
    {
        ui->stackedWidget->setCurrentIndex(1);
    });
    ui->information->setText("未连接");
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_login_clicked()
{
    QString user_ID = ui->ID->text();
    QString user_password = ui->Password->text();
    if(user_ID.isEmpty() || user_password.isEmpty())
    {
        ui->information->setText("用户名或密码不能为空");
        return ;
    }
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
         ui->information->setText("请先连接服务器");
         return;
    }
    Msg msg;
    memset(&msg,0,sizeof(msg));
    msg.type = MSG_LOGIN;
    msg.user_id = user_ID.toInt();
    strncpy(msg.password,user_password.toUtf8().constData(),31);
    ui->information->setText("正在登录...");
    tcpSocket->write((const char *)&msg,sizeof(Msg));

}

void Widget::onConnected()
{
    ui->information->setText("连接成功");
}
void Widget::onErrorOccurred(QAbstractSocket::SocketError)
{
    ui->information->setText("连接失败：" + tcpSocket->errorString());
}

void Widget::on_delete_user_clicked()
{
    int id = ui->ID->text().toInt();
    if(id < 10000 || id > 99999)
    {
        ui->information->setText("请输入有效5位ip");
        return;
    }
    Msg msg;
    memset(&msg,0,sizeof(msg));
    msg.type = MSG_DELETE_USER;
    msg.user_id = id;
    ui->information->setText("正在删除.....");
    tcpSocket->write((const char *)&msg,sizeof(Msg));
}
/*信息接收函数*/
void Widget::onReadyRead()
{
    QByteArray buf = tcpSocket->readAll();
    if(buf.size() < (int)sizeof(Msg)) return ;
    Msg reply;
    memset(&reply,0,sizeof(reply));
    memcpy(&reply, buf.constData(), sizeof(Msg));
    switch (reply.type)
    {
      case MSG_LOGIN:
      {                 /* 登录回复 */
            if (reply.result == RESULT_OK)
            {
               ui->information->setText("登录成功");
               ui->stackedWidget->setCurrentIndex(2);   /* 登录成功才切查询页 */
               ui->Password->clear();
            }
            else if (reply.result == RESULT_USER_NONE)
            {
               ui->information->setText("用户不存在");
            }
            else if (reply.result == RESULT_PWD_ERR)
            {
               ui->information->setText("密码错误");
            }
            else
            {
               ui->information->setText("数据库错误");
            }
       }
       break;
       case MSG_DELETE_USER:
       {           /* 删除用户回复 */
         if (reply.result == RESULT_OK)
         {
             ui->information->setText("删除成功");
             ui->ID->clear();
             ui->Password->clear();
         } else if (reply.result == RESULT_USER_NONE)
         {
             ui->information->setText("用户不存在");
         }
         else
         {
             ui->information->setText("数据库错误");
         }
       }
       break;
      case MSG_REGISTER:  /* 注册回复 → 显示到注册页信息框 */
      {
        if(reply.result == RESULT_OK)
        {
             ui->register_information->setText("注册成功");
             ui->register_ID->clear();
             ui->register_password->clear();
        }
        else if (reply.result == RESULT_USER_EXIST)
             ui->register_information->setText("该ID已注册");
        else if (reply.result == RESULT_INVALID_ID)
             ui->register_information->setText("ID必须为5位数字");
        else
             ui->register_information->setText("数据库错误");
     }
      break;
        case MSG_QUERY:
        {                 /* 查询回复 → 显示到 Chinese 框 */
           if (reply.result == RESULT_FOUND)
              ui->Chinese->setText(QString(reply.chinese));
           else if (reply.result == RESULT_NOT_FOUND)
              ui->Chinese->setText("未找到该单词");
           else
              ui->Chinese->setText("查询失败");
        }
       break;
       default:
            break;
      }
}

void Widget::on_connect_clicked()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState)
    {
        ui->information->setText("已连接，无需重复操作");
        return;
    }
    tcpSocket->connectToHost("192.168.1.140", 8080);
}


void Widget::on_return_2_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
}

void Widget::on_register_3_clicked()
{
    QString idstr = ui->register_ID->text().trimmed();
    QString pwdstr = ui->register_password->text().trimmed();
    if(idstr.isEmpty() || pwdstr.isEmpty())
    {
        ui->register_information->setText("ID或者密码不能为空");
        return;
    }
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        ui->register_information->setText("请先连接服务器");
        return;
    }
    Msg msg;
    memset(&msg, 0, sizeof(msg));
    msg.type    = MSG_REGISTER;                  /* 1 */
    msg.user_id = idstr.toInt();
    strncpy(msg.password, pwdstr.toUtf8().constData(), 31);

    ui->register_information->setText("正在注册...");
    tcpSocket->write((const char *)&msg, sizeof(Msg));
}

void Widget::on_search_clicked()
{
    QString English = ui->English->text();
    if(English.isEmpty())
    {
        ui->search_information->setText("输入不能未空");
        return;
    }
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
    {
        ui->search_information->setText("请先连接服务器");
        return;
    }
    Msg msg;
    memset(&msg,0,sizeof(Msg));
    msg.type = MSG_QUERY;
    strncpy(msg.english,English.toUtf8().constData(),63);
    ui->Chinese->setText("正在查询...");
    tcpSocket->write((const char *)&msg,sizeof(Msg));
}
