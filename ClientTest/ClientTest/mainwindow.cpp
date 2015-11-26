#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QHash<QString, QVariant> status;

    status[QString("test_totalFoci")] = qint16(30);
    status[QString("test_finishedFoci")] = qint16(10);

    client.setStatus(status);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_u_buttonListen_clicked()
{
    client.listen();
}

void MainWindow::on_u_buttonProgress_clicked()
{
    client.send();
}
