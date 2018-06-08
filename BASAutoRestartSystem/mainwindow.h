#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include "comthread.h"
#include "netthread.h"
#include "switchcontrol.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
     void ui_init(void);
     MyThread threadCom;
     NetThread threadNet;
     void sendCOM(QString message);
     //void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
private slots:
     void on_pushButton_open_com_clicked();
     void replyFinished(QNetworkReply *reply);
     void displayRxData();
     void displayNetConnectReturn();
     void on_pushButton_close_com_clicked();

     void on_pushButton_send_clicked();

     void on_pushButton_YV1_close_clicked();

     void on_pushButton_YV1_open_clicked();

     void on_pushButton_YV2_close_clicked();

     void on_pushButton_YV2_open_clicked();

     void on_pushButton_testConnect_clicked();

     void on_pushButton_stop_clicked();

     void on_pushButton_start_clicked();

     void onToggled(bool bChecked);

     void netDisconnect();

     void netConnect();

     void recoverTimeout();

     void recoverNetConnectTest();
private:
    Ui::MainWindow *ui;
    SwitchControl *pSwitchControl;
    int disconnectTime;//判定断网时间（秒）
    uint recordTime;

    //QProcess * cmd;
};

#endif // MAINWINDOW_H
