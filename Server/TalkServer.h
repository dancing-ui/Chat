#ifndef TALKSERVER_H
#define TALKSERVER_H
#include<QTcpServer>
#include<QObject>
#include<QDebug>
#include<QTcpSocket>
#include<QByteArray>
class TalkServer:public QObject{
    Q_OBJECT

public:
    TalkServer()
    {
        m_tcpserver=new QTcpServer(this);
        serverIp="192.168.2.222";
        if(m_tcpserver->listen(QHostAddress("192.168.2.222"),1234)==true)
        {
            qDebug()<<"监听成功";
        }//监听服务器端口查看是否有连接
        connect(m_tcpserver,&QTcpServer::newConnection,this,&TalkServer::dealNewConnection);//建立新链接的初始化
    }
    ~TalkServer()
    {

    }
    void dealNewConnection()
    {
        while(m_tcpserver->hasPendingConnections())
        {
            m_tcpsocket=m_tcpserver->nextPendingConnection();//接受客户端发来的socket，不需要new一个新对象,就相当于客户端的socket
            if(searchIp.find(m_tcpsocket->peerAddress().toString())!=searchIp.end())
            {
                qDebug()<<"Ip已被占用";
                continue;
            }
            else
            {
                QString str = QString("[ip:%1,port:%2]建立新连接").arg(m_tcpsocket->peerAddress().toString()).arg(m_tcpsocket->peerPort());
                qDebug()<<str;

            }
            searchIp.insert(m_tcpsocket->peerAddress().toString(),m_tcpsocket);
            qDebug()<<"现在在线的人数:"<<searchIp.size();
            connect(m_tcpsocket,&QTcpSocket::disconnected,this,[&](){//客户端断开连接时，删除客户端信息
                QTcpSocket* send=static_cast<QTcpSocket*>(sender());
                searchIp.erase(searchIp.find(send->peerAddress().toString()));
                qDebug()<< send->peerAddress().toString()<<"已下线"<<",现在在线的人数:"<<searchIp.size();
            });
            connect(m_tcpsocket,&QTcpSocket::readyRead,this,[=](){//当客户端的socket写数据，也就是发信息的时候需要做什么？
                QTcpSocket* socket=static_cast<QTcpSocket*>(sender());
                QByteArray data;
                data=socket->readAll();//读取客户端发来的数据
                if(searchIp.find(QString(data.mid(5,data[4])))!=searchIp.end())
                {
                    senderSocket=*searchIp.find(QString(data.mid(5,data[4])));
                    qDebug()<<"发送方IP"<<senderSocket->peerAddress().toString();
                    dealSend(data);
                }
                else
                {
                    qDebug()<<"发送方未上线";
                }

                if(searchIp.find(QString(data.mid(5+data[4]+1,data[4+data[4]+1])))!=searchIp.end())
                {
                    receiverSocket=*searchIp.find(QString(data.mid(5+data[4]+1,data[4+data[4]+1])));
                    qDebug()<<"接收方IP"<<receiverSocket->peerAddress().toString();
                    dealReceive(data);
                }
                else
                {
                    qDebug()<<"接收方未上线";
                }
            });//建立读取包的信号
        }

    }
    void dealSend(QByteArray& data)
    {
        QByteArray msg;
        quint16 l=QString(data.mid(5+data[4]+1+data[4+data[4]+1])).size()+senderSocket->peerAddress().toString().size()+serverIp.size()+6;
        msg.append(0x66);
        msg.append(static_cast<char>(l&0x00FF));
        msg.append(static_cast<char>(l>>8&0x00FF));
        msg.append(0x02);
        msg.append(serverIp.size());//data[4]
        msg.append(serverIp);
        msg.append(senderSocket->peerAddress().toString().size());//
        msg.append(senderSocket->peerAddress().toString());
        int i;
        for(i=5+data[4]+1+data[4+data[4]+1];static_cast<char>(data[i])!=0x05;i++)
        {
            msg.append(data[i]);
        }
        msg.append(0x05);
        msg.append(data.mid(i+1));

        qDebug()<<"发送方IP："<<QString(data.mid(5,data[4]))<<" ";
        qDebug()<<"接收方IP："<<QString(data.mid(5+data[4]+1,data[4+data[4]+1]));
        qDebug()<<"发送的信息："<<QString(data.mid(i+1));

        senderSocket->write(msg);
        bool iswrite = senderSocket->waitForBytesWritten();
        if (iswrite)
        {
            qDebug()<<"服务器写入发送方成功";
        }
        else
        {
            qDebug()<<"服务器写入发送方失败";
        }
    }
    void dealReceive(QByteArray& data)
    {
        QByteArray msg;
        quint16 l=QString(data.mid(5+data[4]+1+data[4+data[4]+1])).size()+receiverSocket->peerAddress().toString().size()+serverIp.size()+6;
        msg.append(0x66);
        msg.append(static_cast<char>(l&0x00FF));
        msg.append(static_cast<char>(l>>8&0x00FF));
        msg.append(0x03);
        msg.append(serverIp.size());//data[4]
        msg.append(serverIp);
        msg.append(receiverSocket->peerAddress().toString().size());
        msg.append(receiverSocket->peerAddress().toString());
        int i;
        for(i=5+data[4]+1+data[4+data[4]+1];static_cast<char>(data[i])!=0x05;i++)
        {
            msg.append(data[i]);
        }
        msg.append(0x05);
        msg.append(data.mid(i+1));

        qDebug()<<"发送方IP："<<QString(data.mid(5,data[4]))<<" ";
        qDebug()<<"接收方IP："<<QString(data.mid(5+data[4]+1,data[4+data[4]+1]));
        qDebug()<<"发送的信息："<<QString(data.mid(i+1));

        receiverSocket->write(msg);
        bool iswrite = receiverSocket->waitForBytesWritten();
        if (iswrite)
        {
            qDebug()<<"服务器写入接收方成功";
        }
        else
        {
            qDebug()<<"服务器写入接收方失败";
        }
    }
private:
    QTcpServer* m_tcpserver;
    QTcpSocket* m_tcpsocket;
    QString serverIp;
    QHash<QString,QTcpSocket*> searchIp;
    QTcpSocket* senderSocket;
    QTcpSocket* receiverSocket;
};
#endif // TALKSERVER_H
