#include "comthread.h"
#include <QtDebug>
//FOR RS232
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

MyThread::MyThread()
{
    stopped=false;
    com_opened = false;
}

MyThread::~MyThread()
{
    //delete my_serialport;
    stop();
    quit();//优雅地结束线程，必备！！
    wait();//优雅地结束线程，必备！！
}

void MyThread::run()
{
    QSerialPort *my_serialport = new QSerialPort;

    while(!stopped)
    {
        if(stopped&&com_opened)
        {
            my_serialport->close();
            com_opened=false;
        }
        if(!com_opened)
        {
            //open com
            qDebug() << "Brush:" <<"---open com port------";
            com_opened=true;
            my_serialport->setPortName(portnum);
            my_serialport->open(QIODevice::ReadWrite);
            my_serialport->setBaudRate(buadRate.toInt());
            my_serialport->setDataBits(QSerialPort::Data8);
            my_serialport->setParity(QSerialPort::NoParity);
            my_serialport->setStopBits(QSerialPort::OneStop);
            my_serialport->setFlowControl(QSerialPort::NoFlowControl);
            com_opened=true;
        }
        if(this->com_opened&&this->tx_event)
        {
            this->tx_event=false;
            my_serialport->clear(QSerialPort::AllDirections);
            //qDebug() << "Brush:" <<"send data to com2"<<this->TxData.length();
            //qDebug() << "arr size:" <<this->TxData.length();
            my_serialport->write(this->TxData);
            if (my_serialport->waitForBytesWritten(5))
            {
                //qDebug() << "Brush:" <<"send data success";
                if (my_serialport->waitForReadyRead(1500))  //1s
                {
                    requestData = my_serialport->readAll();
                    while (my_serialport->waitForReadyRead(15))
                        requestData += my_serialport->readAll();
                    emit(this->comRecive());
                }else
                {
                    qDebug() << "Brush:" <<"wait return time out";
                }
            }else
            {
                qDebug() << "Brush:" <<"send time out";
            }
        }
        if (my_serialport->waitForReadyRead(5))  //50ms
        {
            while (my_serialport->waitForReadyRead(5))
                this->msleep(20);
            requestData = my_serialport->readAll();
            emit(this->comRecive());
        }
        if(stopped&&com_opened)
        {
            my_serialport->close();
            com_opened=false;
        }
    }
}

void MyThread::stop()
{
    stopped=true;

}
void MyThread::startCom()
{
    stopped=false;

}
void MyThread::changeComState(bool stat)
{
    com_opened=stat;
}
void MyThread::setMessage(const QString &message)
{
    messageStr = message;

}
void MyThread::setPortnum(const QString &num)
{
    portnum=num;

}
void MyThread::setBuadRate(const QString &rate)
{
    buadRate=rate;

}
void MyThread:: changeTxState(bool stat)
{
    tx_event=stat;
}
void MyThread:: changeRxState(bool stat)
{
    rx_event=stat;
}
