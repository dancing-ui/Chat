#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include<QRect>
#include<QAbstractSocket>
#include<QTcpSocket>
#include<QByteArray>
#include<QAbstractButton>
#include<QSqlDatabase>
#include<QToolButton>
#include<QMap>
#include<QHostAddress>
#include<QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    void openWidget();
    void tcpConnect();


    QHostAddress localIp;
    QHostAddress serverIp;
    QHostAddress friendIp;
    uint16_t port;

    bool normal;
    uint32_t user_id;
    QString userName;
    uint32_t friend_id;
    QString friendName;
    void connectDB();
    void getFriend();
    void setName();
    QSqlDatabase db;
    QTimer *newMsg;

    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
private slots:
    void dealClient();

    void on_toolButton_magnify_clicked();

    void on_toolButton_shrink_clicked();

    void on_toolButton_close_clicked();



    void on_toolButton_message_send_clicked();

    void on_toolButton_clicked();

private:
    Ui::Widget *ui;

    QTcpSocket* m_tcp;//接受信息的端口

    bool pressed;
    QPoint point_wnd;
    QPoint point_press;
    void keyPressEvent(QKeyEvent *e);



    void getMessage(uint32_t friend_id,QString friendName);

    class DeffieHellman
    {
        uintmax_t p,secKey,pubKey;
        void init();
    };
    QHash<uint16_t,QToolButton*> searchFriend;
    QString getName(uint16_t id);

};

#endif // WIDGET_H
