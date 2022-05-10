#include "widget.h"
#include "ui_widget.h"
#include<QPushButton>
#include<QMouseEvent>
#include<QDebug>
#include<QHostAddress>
#include<QTimer>
#include<QTextEdit>
#include<QSqlDatabase>
#include<QScrollArea>
#include <QSqlQuery>
#include <QScrollBar>
#include <QDateTime>
#include<QListWidget>
#include <QMessageBox>
#include<QHashIterator>
#include<bitset>
using namespace std;
Widget::Widget(QWidget *parent)
    : QWidget(parent),
     ui(new Ui::Widget)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);//隐藏边框
    this->setFocusPolicy(Qt::StrongFocus);//接受按键事件

    ui->toolButton_close->setIcon(QIcon(":/Image/close.svg"));
    ui->toolButton_shrink->setIcon(QIcon(":/Image/shrink.svg"));
    ui->toolButton_magnify->setIcon(QIcon(":/Image/magnify.svg"));

    ui->stackedWidget->setCurrentIndex(1);
    this->setWindowIcon(QIcon(":/Image/Tchat.svg"));

    newMsg=new QTimer(this);
    connect(newMsg, &QTimer::timeout, this, [=](){
        connectDB();
        QSqlQuery query(db);
        QString cmd="select count(sender_id),sender_id from message where receiver_id=%1 and readed=0 group by sender_id";
        query.exec(cmd.arg(user_id));
        while(query.next())
        {
            QString cnt=query.value(0).toString();
            uint16_t sender_id=query.value(1).toUInt();
            if(cnt>0&&friend_id!=sender_id)
            {
                searchFriend[sender_id]->setStyleSheet(QString("background-color: rgb(255,0,0)"));
                searchFriend[sender_id]->setText(getName(sender_id)+" 有"+cnt+"条未读消息");
            }
        }
        db.close();
    });
}

Widget::~Widget()
{
    delete ui;
}

void Widget::tcpConnect()
{
    qDebug()<<"server IP:"<<serverIp;
    qDebug()<<"local IP:"<<localIp;
    qDebug()<<"socket:"<<port;

    m_tcp=new QTcpSocket(this);

    if(m_tcp->bind(static_cast<QHostAddress>(localIp))==true)
    {
        qDebug()<<"IP绑定成功";
    }
    m_tcp->connectToHost(static_cast<QHostAddress>(serverIp),port);
    if(m_tcp->waitForConnected(5000)==true)//解决网络不同步连接的问题
    {
        ui->textBrowser_chatBox->append(localIp.toString()+"已和服务器连接成功");
        connect(m_tcp,&QAbstractSocket::disconnectFromHost,[&](){
            ui->textBrowser_chatBox->append(localIp.toString()+"已和服务器断开连接");
        });
        connect(m_tcp,&QAbstractSocket::readyRead,this,&Widget::dealClient);//读取接收的包，并进行处理
    }
    else
    {
        ui->textBrowser_chatBox->clear();
        ui->textBrowser_chatBox->append("服务器连接中，请稍后...");
    }
}

void Widget::dealClient()//处理接受到的包
{
    QByteArray data=m_tcp->readAll();

    QByteArray sender;
    uint16_t sender_id=0;
    int i;
    for(i=5+data[4]+1+data[4+data[4]+1];static_cast<char>(data[i])!=0x05;i++)
    {
        sender.append(data[i]);
    }
    for(auto i=sender.rbegin();i!=sender.rend();i++)
    {
        sender_id=sender_id*10+(*i-'0');
    }
    qDebug()<<"发送方ID："<<sender_id;
    QString str=QString(data.mid(i+1));
    qDebug()<<"发送的信息:"<<str;

    connectDB();
    QSqlQuery query(db);
    QString com="insert into message(sender_id,receiver_id,content,time,readed) values(%1,%2,'%3','%4',0);";
    QString date=QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");//插入日期数据要指明类型
    char cmd=data[3];
    if(cmd==0x02)
    {
        query.exec(com.arg(user_id).arg(friend_id).arg(str).arg(date));
        db.close();
        getMessage(friend_id,friendName);
    }
    else if(cmd==0x03)
    {
        if(friend_id==sender_id)
        {
            getMessage(sender_id,getName(sender_id));
        }
    }
}

void Widget::on_toolButton_message_send_clicked()
{
    if(m_tcp->state()==QAbstractSocket::UnconnectedState)
    {
        ui->textBrowser_chatBox->clear();
        ui->textBrowser_chatBox->append("网络连接已断开");
        return;
    }
    auto str=ui->textEdit_inputBox->toPlainText();
    if(!str.isEmpty())
    {
        connectDB();
        QSqlQuery query(db);
        query.exec(QString("select IP from user where user_id=%1").arg(friend_id));
        if(query.size()==0)
        {
            return;
        }
        while(query.next())
        {
            friendIp = static_cast<QHostAddress>(query.value(0).toString());
        }
        db.close();
        qDebug()<<"要发送的朋友IP是"<<friendIp;

        ui->textEdit_inputBox->clear();

        QByteArray data;
        quint16 l=str.size()+localIp.toString().size()+friendIp.toString().size()+6+QString(user_id).size()+1;
        data.append(0x66);
        data.append(static_cast<char>(l&0x00FF));
        data.append(static_cast<char>(l>>8&0x00FF));
        data.append(0x03);
        data.append(localIp.toString().size());//data[4]
        data.append(localIp.toString());
        data.append(friendIp.toString().size());
        data.append(friendIp.toString());

        QString sender_id;
        uint16_t sender=user_id;
        qDebug()<<"用户ID:"<<user_id;
        while(sender)
        {
            sender_id.push_back(sender%10+'0');
            sender/=10;
        }

        data.append(sender_id);
        data.append(0x05);

        data.append(str.toUtf8());

        qDebug()<<"local Ip:"<<QString(data.mid(5,data[4]))<<" ";
        qDebug()<<"friend Ip:"<<QString(data.mid(5+data[4]+1,data[4+data[4]+1]));
        qDebug()<<"发送者Id:";
        int i;
        for(i=5+data[4]+1+data[4+data[4]+1];static_cast<char>(data[i])!=0x05;i++)
        {
            qDebug()<<data[i];
        }
        qDebug()<<"信息:"<<QString(data.mid(i+1));

        m_tcp->write(data);//向服务器发送包
        bool iswrite = m_tcp->waitForBytesWritten();
        if (iswrite)
        {
            qDebug()<<"写入成功";
        }
        else
        {
            qDebug()<<"写入失败";
        }
   }
}
void Widget::keyPressEvent(QKeyEvent *e)
{
    if(e->key()==Qt::Key_Backspace)
    {
        on_toolButton_message_send_clicked();
    }
}

void Widget::connectDB()
{
    qDebug()<<"我打开了数据库";
    db = QSqlDatabase::addDatabase("QMYSQL");  //连接的MYSQL的数据库驱动
    db.setHostName("127.0.0.1");         //主机名
    db.setPort(3306);                    //端口
    db.setDatabaseName("chat");          //数据库名
    db.setUserName("root");              //用户名
    db.setPassword("123456");            //密码
    db.open();
    //测试连接
    if(!db.open())
    {
        qDebug() << "数据库连接失败";
    }
    else
    {
         qDebug() << "数据库连接成功";
    }
}

void Widget::getFriend()
{
    connectDB();
    QSqlQuery query(db);
    QString str=QString("select user_id,user_name from user where user_id in (select user2_id from user_relation where user1_id=%1 union all select user1_id from user_relation where user2_id=%1)").arg(user_id);
    query.exec(str);

    //scrollArea添加控件：先布局，再添加
    QGridLayout *pLayout = new QGridLayout();//网格布局

    QLabel* title=new QLabel("好友");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(QString("background-color: rgb(83, 255, 250)"));
    title->setMinimumSize(QSize(322,30));   //width height
    title->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);

    pLayout->addWidget(title);
    QWidget * pWgt = new QWidget;
    uint8_t cnt=0;

    while(query.next())
    {
        uint32_t btn_id=query.value(0).toString().toUInt();
        QString btn_name=query.value(1).toString();
        QToolButton* btn=new QToolButton;
        btn->setText(btn_name);
        btn->setMinimumSize(QSize(322,40));   //width height
        btn->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        btn->setAutoRaise(true);//鼠标一放上去就出现按钮框
        btn->setStyleSheet(QString("background-color: rgb(255,255,255)"));
        searchFriend[btn_id]=btn;
        connect(btn,&QToolButton::clicked,this,[=](){
            friend_id=btn_id;
            friendName=btn_name;
            btn->setText(btn_name);
            btn->setStyleSheet(QString("background-color: rgb(255,255,255)"));
            getMessage(friend_id,friendName);
        });
        pLayout->addWidget(btn);//把按钮添加到布局控件中
        cnt++;
    }
    pWgt->setLayout(pLayout);
    ui->scrollArea_friend->setWidget(pWgt);
    db.close();
}

void Widget::setName()
{
    ui->label_userName->setText(userName);
}

void Widget::getMessage(uint32_t friend_id,QString friendName)
{
    ui->textBrowser_chatBox->clear();
    connectDB();

    ui->label_name->setText(friendName);
    QSqlQuery query(db);
    QString str=QString("select * from message where sender_id=%1 and receiver_id=%2 union all select * from message where sender_id=%2 and receiver_id=%1 order by time").arg(user_id).arg(friend_id);
    query.exec(QString("update message set readed=1 where receiver_id=%1 and sender_id=%2").arg(user_id).arg(friend_id));
    query.exec(str);

    QDate previousDate(1999,1,1);

    while(query.next())
    {
        uint32_t sender=query.value(1).toString().toUInt();
        QString content=query.value(3).toString();
        QDateTime time=query.value(4).toDateTime();

        if(previousDate!=time.date())
        {
            previousDate=time.date();
            ui->textBrowser_chatBox->append("\n\t\t\t     "+previousDate.toString("yyyy-MM-dd")+"\n");
        }
        if(sender==user_id)
        {
            ui->textBrowser_chatBox->append(userName+"  "+time.time().toString("HH:mm:ss")+"\n");
            ui->textBrowser_chatBox->append(content.toUtf8()+"\n");
        }
        else
        {
            ui->textBrowser_chatBox->append(friendName+"  "+time.time().toString("HH:mm:ss")+"\n");
            ui->textBrowser_chatBox->append(content.toUtf8()+"\n");
        }
    }
    db.close();
}

QString Widget::getName(uint16_t id)
{
    connectDB();
    QSqlQuery query(db);
    query.exec(QString("select user_name from user where user_id=%1").arg(id));
    while(query.next())
    {
        return query.value(0).toString();
    }
    db.close();
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    QRect r(207,0,681,40);//记录当前名字栏的相对位置
    auto p=event->pos();//记录当前鼠标位置

    if(r.contains(p))//如果鼠标没点标题栏是移不动的
    {
        pressed=true;
        point_wnd=this->geometry().topLeft();//记录整个窗口左上角的点
        point_press=this->mapToGlobal(p);//记录鼠标按下点对于整个屏幕来说的位置
    }

}
void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    pressed=false;//释放之后就不能再移动了
}
void Widget::mouseMoveEvent(QMouseEvent *event)
{
    if(pressed)//只有按下并移动时，才触发事件
    {
        auto v=mapToGlobal(event->pos())-point_press;//将当前鼠标的绝对位置与之前的绝对位置相减得到一个偏移量
        auto r=point_wnd;//获取之前整个窗口的位置
        this->move(r+v);//窗口位置加上偏移量，就可以移动窗口到新位置
    }
}


void Widget::on_toolButton_magnify_clicked()
{
    if(normal==false)
    {

        ui->toolButton_magnify->setIcon(QIcon(":/Image/normal.svg"));
        this->showMaximized();
        normal=true;
    }
    else
    {
        ui->toolButton_magnify->setIcon(QIcon(":/Image/magnify.svg"));
        this->showNormal();
        normal=false;
    }
}

void Widget::on_toolButton_shrink_clicked()
{
    this->showMinimized();
}

void Widget::on_toolButton_close_clicked()
{
    m_tcp->disconnectFromHost();//与服务器端断开连接
    m_tcp->close();//关闭socket
    this->close();
}

void Widget::on_toolButton_clicked()
{
    connectDB();
    QSqlQuery query(db);
    query.exec(QString("select user_name,sex,birthday from user where user_id=%1").arg(ui->lineEdit_search->text()));
    if(query.size()==0)
    {
        QMessageBox::information(NULL, "提示", "该用户不存在", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    while(query.next())
    {
        QString name=query.value(0).toString();
        ui->label_name->setText(name);
        friend_id=ui->lineEdit_search->text().toUInt();
        friendName=name;
        getMessage(friend_id,friendName);
    }
    db.close();
}
