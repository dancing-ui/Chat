#include "settings.h"
#include "ui_settings.h"
#include<QHostAddress>
#include<QNetworkInterface>
settings::settings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::settings)
{
    ui->setupUi(this);
    getAllIp();
}

settings::~settings()
{
    delete ui;
}
QHostAddress settings::getLocalIP()
{
    return static_cast<QHostAddress>(ui->comboBox_localIP->currentText()) ;
}
QHostAddress settings::getSeverIP()
{
    return static_cast<QHostAddress>(ui->lineEdit_server->text());
}
int settings::getPort()
{
    return ui->lineEdit_socket->text().toUInt();
}

void settings::on_pushButton_clicked()
{

    this->close();
}

void settings::on_pushButton_2_clicked()
{
    this->close();
}

void settings::getAllIp()
{
    ui->comboBox_localIP->clear();
    QList<QHostAddress> list=QNetworkInterface::allAddresses();
    for(auto& i:list)
    {
        if(i.isGlobal())
        {
            ui->comboBox_localIP->addItem(i.toString());
        }
    }
    ui->lineEdit_server->setText("192.168.2.222");
    ui->lineEdit_socket->setText("1234");
}
