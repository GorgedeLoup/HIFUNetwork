#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Server.h"
#include <QMainWindow>
#include <QTime>
#include <QTimer>


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
    Server server;
    QTime m_time;
    QTimer *m_checkTimer;
    QTimer *m_testTimer;

    qint16 m_testTimes;

    QHash<float, QList<Spot3DCoordinate> > spot;
    QHash<float, QList<int> > spotOrder;
    SpotSonicationParameter parameter;


private slots:
    int randomNum();
    void numerousTest(int times);
    void resetInput();

    void setCheckTimer();
    void checkTimerSlot();

    void setTestTimer();
    void testTimerSlot();

    void testFun();

    void on_u_buttonTP_clicked();
    void on_u_buttonTest_clicked();
    void on_u_buttonListen_clicked();
    void on_u_buttonStart_clicked();
    void on_u_buttonStop_clicked();
    void on_u_buttonPause_clicked();
    void on_u_buttonResume_clicked();
    void on_u_buttonCheckConnection_clicked();
    void on_u_buttonNumTest_clicked();
};

#endif // MAINWINDOW_H
