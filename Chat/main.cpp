#include "widget.h"

#include <QApplication>
#include<login.h>
#include <QLibrary>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LogIn di;
    di.show();
    di.playVideo();
    return a.exec();
}
