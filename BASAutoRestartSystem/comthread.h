#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QThread>
#include <stdio.h>
#include <QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
class MyThread : public QThread
{
    Q_OBJECT
public:
    QByteArray requestData;
    QByteArray TxData;
    MyThread();
    ~MyThread();
    void setMessage(const QString &message);
    void setPortnum(const QString &num);
    void setBuadRate(const QString &rate);
    void stop();
    void startCom();
    void changeTxState(bool stat);
    void changeRxState(bool stat);
    void changeComState(bool stat);


    volatile bool com_opened;
signals:
   // void request(const QString &s);
    void comRecive();



protected:
    void run();

private:
    QString messageStr;
    QString portnum;
    QString buadRate;


    volatile bool stopped;
    volatile bool tx_event;
    volatile bool rx_event;
};

#endif // MYTHREAD_H
