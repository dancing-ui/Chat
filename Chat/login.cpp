#include "login.h"
#include "ui_login.h"
#include<QPushButton>
#include<widget.h>
#include<QList>
#include<QNetworkInterface>
#include<QDebug>
#include<QLineEdit>
#include<QComboBox>
#include<QLabel>
#include<QSqlDatabase>
#include<QSqlQuery>
#include <QMessageBox>
#include"settings.h"
#include <QDateTime>
#include<QMovie>
#include<QMediaPlayer>
LogIn::LogIn(QWidget *parent) : QWidget(parent),
    ui(new Ui::LogIn)
{
    ui->setupUi(this);
    this->setWindowTitle("EChat");
    this->setWindowIcon(QIcon(":/Image/Tchat.svg"));

    video_recur=true;
    qDebug()<<QSqlDatabase::drivers();

    connect(ui->lineEdit_account,&QLineEdit::textChanged,this,&LogIn::checkAccount);
    connect(ui->lineEdit_password,&QLineEdit::textChanged,this,&LogIn::checkPassword);
    passed=false;
    visibled=false;
    ui->toolButton_password->setIcon(QIcon(":/Image/passvisible.svg"));
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);

    QDateTime date=QDateTime::currentDateTime();

    ui->label_date->setAlignment(Qt::AlignCenter);
    ui->label_date->setText(date.toString("yyyy / MM / dd "));
}
void LogIn::checkAccount()
{
    ui->label_account->clear();
    ui->label_password->clear();
    connectDB();
    QSqlQuery query(db);
    QString ac= ui->lineEdit_account->text();
    query.exec(QString("select user_id from user where user_id=%1").arg(ac));
    int cnt=0;
    while(query.next())
    {
        account=query.value(0).toString();
        qDebug()<<"账号: "<<account;
        cnt++;
    }
    if(cnt==0)
    {
        ui->label_account->setText("账号不存在");
    }
    db.close();
}

void LogIn::checkPassword()
{
    if(ui->lineEdit_account->text().isEmpty())
    {
        ui->label_account->setText("请先输入账号");
        return;
    }
    ui->label_account->clear();
    ui->label_password->clear();

    connectDB();
    QSqlQuery query(db);
    QString ac= ui->lineEdit_account->text(),pass=ui->lineEdit_password->text();
    query.exec(QString("select password from user where user_id=%1").arg(ac.toUInt()));
    int cnt=0;
    while(query.next())
    {
        password=query.value(0).toString();
        qDebug()<<password;
        cnt++;
    }

    if(pass==password)
    {
        passed=true;
    }
    else if(pass!=password)
    {
        ui->label_password->setText("密码错误");
    }
    db.close();
}

void LogIn::playVideo()
{
    qDebug()<<"开始播放视频";
    player=new QMediaPlayer;
    videoWidget=new QVideoWidget;
    player->setVideoOutput(videoWidget);
    player->setMedia(QUrl::fromLocalFile("C:/Users/惠普/Videos/your_name.mp4"));

    QGridLayout *layout=new QGridLayout;
    layout->addWidget(videoWidget);
    ui->widget_video->setLayout(layout);

    ui->widget_video->show();
    player->play();
    connect(player,&QMediaPlayer::stateChanged,this,[=](){
        if(player->mediaStatus()==QMediaPlayer::EndOfMedia&&video_recur==true)
        {
            player->play();
        }
    });
}

LogIn::~LogIn()
{
    delete ui;
}
void LogIn::openWidget()
{
    w.localIp=setting.getLocalIP();
    w.serverIp=setting.getSeverIP();
    w.port=setting.getPort();
    w.tcpConnect();//建立tcp连接
    w.normal=false;
    w.user_id=ui->lineEdit_account->text().toUInt();
    w.getFriend();

    w.connectDB();

    QSqlQuery query(w.db);
    query.exec(QString("select user_name from user where user_id=%1").arg(w.user_id));
    while(query.next())
    {
        w.userName=query.value(0).toString();
    }
    w.setName();
    qDebug()<<"现在的IP地址和用户id是"<<w.localIp.toString()<<" "<<w.user_id;
    QString qq= QString("update user set IP = '%1' where user_id= %2 ").arg(w.localIp.toString()).arg(w.user_id);
    //太恶心了，%1要用单引号引起来,字符要用单引号引起来，所有%1都要用单引号引起来
    if(query.exec(qq)==true)
    {
        qDebug()<<"修改IP成功";
    }
    db.close();

    player->stop();
    w.newMsg->start(2000);
    w.show();
    this->close();
}

void LogIn::on_pushButton_settings_clicked()
{
    setting.getAllIp();
    setting.setWindowTitle("网络连接设置");
    setting.setWindowIcon(QIcon(":/Image/settings.svg"));
    setting.show();
}

void LogIn::on_toolButton_password_clicked()
{
    if(visibled==false)
    {
        visibled=true;
        ui->toolButton_password->setIcon(QIcon(":/Image/passnotvisible.svg"));
        ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
    }
    else
    {
        visibled=false;
        ui->toolButton_password->setIcon(QIcon(":/Image/passvisible.svg"));
        ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    }
}

void LogIn::on_toolButton_logIn_clicked()
{
    qDebug()<<"ok";
    if(passed)
    {
        db.close();//关闭数据库连接
        video_recur=false;
        player->stop();
        openWidget();//打开客户端
    }
}

void LogIn::connectDB()
{
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
        qDebug() << "连接失败";
    }
    else
    {
         qDebug() << "连接成功";
    }
}
