#ifndef SETTINGS_H
#define SETTINGS_H

#include <QWidget>
#include<QHostAddress>
namespace Ui {
class settings;
}

class settings : public QWidget
{
    Q_OBJECT

public:
    explicit settings(QWidget *parent = nullptr);
    ~settings();
    QHostAddress getLocalIP();
    QHostAddress getSeverIP();
    int getPort();
    void getAllIp();
private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();


private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
