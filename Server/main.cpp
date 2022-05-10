#include <QCoreApplication>

#include"TalkServer.h"
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    TalkServer talkserver;
    return a.exec();
}
