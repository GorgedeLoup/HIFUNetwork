#include "mainwindow.h"
#include "ui_mainwindow.h"

#define CHECK_DELAY 30000
#define TEST_DELAY 500
#define TEST_TIMES 500


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_checkTimer = new QTimer;
    m_testTimer = new QTimer;

    m_testTimes = 0;

    Spot3DCoordinate spot1, spot2, spot3, spot4, spot5;
    spot1.x = 11; spot1.y = 12; spot1.z = 13;
    spot2.x = 21; spot2.y = 22; spot2.z = 23;
    spot3.x = 31; spot3.y = 32; spot3.z = 33;
    spot4.x = 41; spot4.y = 42; spot4.z = 43;
    spot5.x = 51; spot5.y = 52; spot5.z = 53;
    QList<Spot3DCoordinate> spotList45;
    spotList45 << spot1 << spot2 << spot3 << spot4 << spot5;
    spot[45.0] = spotList45;

    Spot3DCoordinate spot6, spot7, spot8, spot9;
    spot6.x = 1.1; spot6.y = 1.2; spot6.z = 1.3;
    spot7.x = 2.1; spot7.y = 2.2; spot7.z = 2.3;
    spot8.x = 3.1; spot8.y = 3.2; spot8.z = 3.3;
    spot9.x = 4.1; spot9.y = 4.2; spot9.z = 4.3;
    QList<Spot3DCoordinate> spotList90;
    spotList90 << spot6 << spot7 << spot8 << spot9;
    spot[90.0] = spotList90;

    QList<int>  spotOrderList1, spotOrderList2;
    spotOrderList1 << 1 << 2 << 3 << 4 << 5;
    spotOrderList2 << 10 << 9 << 8 << 7 << 6;
    spotOrder[45.0] = spotOrderList1;
    spotOrder[90.0] = spotOrderList2;

    parameter.volt = 5;
    parameter.totalTime = 10;
    parameter.period = 500;
    parameter.dutyCycle = 50;
    parameter.coolingTime = 5;

    resetInput();

    connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(checkTimerSlot()));
    connect(m_testTimer, SIGNAL(timeout()), this, SLOT(testTimerSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}


int MainWindow::randomNum()
{
    m_time = QTime::currentTime();
    qsrand(m_time.msec() + m_time.second()*1000);

    int randNum = qrand() % 4;
    return randNum;
}


void MainWindow::numerousTest(int times)
{   
    if (m_testTimes < times)
    {
        int randNum = randomNum();
        switch (randNum) {
        case 0:
            resetInput();
            server.sendPlan();
            break;
        case 1:
            server.sendCommand(START);
            break;
        case 2:
            server.sendCommand(STOP);
            break;
        case 3:
            server.sendCommand(PAUSE);
            break;
        case 4:
            server.sendCommand(RESUME);
            break;
        default:
            break;
        }
        m_testTimes += 1;
    }
    else{
        m_testTimer->stop();
        m_testTimes = 0;
    }
}


void MainWindow::resetInput()
{
    server.setCoordinate(spot);
    server.setSpotOrder(spotOrder);
    server.setParameter(parameter);
}


void MainWindow::setCheckTimer()
{
    m_checkTimer->setSingleShot(false);
    m_checkTimer->setInterval(CHECK_DELAY);    //ms
    server.checkConnection();
    m_checkTimer->start();
}


void MainWindow::checkTimerSlot()
{
    server.checkConnection();
}


void MainWindow::setTestTimer()
{
    m_testTimer->setSingleShot(false);
    m_testTimer->setInterval(TEST_DELAY);
    numerousTest(TEST_TIMES);
    m_testTimer->start();
}


void MainWindow::testTimerSlot()
{
    numerousTest(TEST_TIMES);
}


void MainWindow::testFun()
{
    server.sendPlan();
    server.sendCommand(START);
    server.checkConnection();
}


void MainWindow::on_u_buttonTP_clicked()
{
    resetInput();
    server.sendPlan();
}

void MainWindow::on_u_buttonTest_clicked()
{
    testFun();
}

void MainWindow::on_u_buttonListen_clicked()
{
    server.listen();
}

void MainWindow::on_u_buttonStart_clicked()
{
    server.sendCommand(START);
}

void MainWindow::on_u_buttonStop_clicked()
{
    server.sendCommand(STOP);
}

void MainWindow::on_u_buttonPause_clicked()
{
    server.sendCommand(PAUSE);
}

void MainWindow::on_u_buttonResume_clicked()
{
    server.sendCommand(RESUME);
}

void MainWindow::on_u_buttonCheckConnection_clicked()
{
    setCheckTimer();
}

void MainWindow::on_u_buttonNumTest_clicked()
{
    setTestTimer();
}
