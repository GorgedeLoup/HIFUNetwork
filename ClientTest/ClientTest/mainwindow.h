#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Client.h"
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
    Ui::MainWindow *ui;
    Client client;


private slots:
    void on_u_buttonListen_clicked();

    void on_u_buttonProgress_clicked();
};

#endif // MAINWINDOW_H
