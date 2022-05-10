#ifndef LOGIN_H
#define LOGIN_H

#include <QWidget>
#include<widget.h>
#include<QSqlDatabase>
#include"settings.h"
#include<QMediaPlayer>
#include<QVideoWidget>
namespace Ui {
class LogIn;
}

class LogIn : public QWidget
{
    Q_OBJECT

public:
    explicit LogIn(QWidget *parent = nullptr);
    ~LogIn();
    void openWidget();
    void checkAccount();
    void checkPassword();
    void playVideo();
private slots:
    void on_pushButton_settings_clicked();

    void on_toolButton_password_clicked();

    void on_toolButton_logIn_clicked();

private:
    Ui::LogIn *ui;
    Widget w;
    QSqlDatabase db;
    settings setting;
    QString account;
    QString password;
    bool passed;
    bool visibled;
    bool video_recur;
    QMediaPlayer *player;
    QVideoWidget *videoWidget;
    void connectDB();
};

#endif // LOGIN_H
