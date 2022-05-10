#include "search.h"
#include "ui_search.h"
#include<QPushButton>
search::search(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::search)
{
    ui->setupUi(this);
    ui->toolButton_search->setIcon(QIcon(":/Image/search.svg"));
}

search::~search()
{
    delete ui;
}
