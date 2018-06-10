#include "netthread.h"
#include <QMessageBox>
#include <QDebug>
#include <QProcess> //使用CMD指令
#include <QtNetwork>
#include <QDebug>
NetThread::NetThread()
{
    //testTimer = new QTimer;
}

NetThread::~NetThread()
{
    //delete  codec;
//    if(testTimer->isActive()){
//        testTimer->stop();
//    }
    requestInterruption();
    quit();
    wait();
    if(testTimer!=NULL){
       delete testTimer;
    }

    // 请求终止

}

void NetThread::stopTimer()
{
//    if(testTimer->isActive()){
//        testTimer->stop();
//        qDebug()<<"执行 testTimer->stop();";
//    }
    //以下语句，停止线程
    requestInterruption();
    quit();
    wait();
    //delete testTimer;

}

void NetThread::startTimer()
{
    if(!testTimer->isActive()){
        testTimer->start();
        qDebug()<<"执行 testTimer->start();";
    }
}

void NetThread::net_connect()
{
    if(!stopped){
        //测试使用CMD指令ping访问IP
        QProcess  cmd;
        cmd.start("ping "+netConnectStr);
        cmd.waitForReadyRead();
        cmd.waitForFinished();
        netConnectReturnStr = QTextCodec::codecForName("gbk")->toUnicode(cmd.readAll());
        bool b = netConnectReturnStr.contains("100% 丢失")||netConnectReturnStr.contains("100% loss");
        /*测试使用HTTP协议访问IP*/
        //    QNetworkAccessManager * mgr = new QNetworkAccessManager(this);
        //    QNetworkRequest url ;
        //    url.setUrl(QUrl("http://192.168.0.1"));
        //    mgr->get(url);
        //    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
        if(b){
            qDebug()<< "网络测试：网络不通！！";
            emit(this->netDisconnect());
        }
        else
        {
            emit(this->netConnect());
        }
    }


    //如果不通，发送信号给主程序。
//    if(b){
//       emit(this->netConnectRecive());
//    }
}

void NetThread::run()
{
//    while(!stopped)
//    {
//        //测试使用CMD指令ping访问IP
//        QProcess  cmd;
//        cmd.start("ping "+netConnectStr);
//        cmd.waitForReadyRead();
//        cmd.waitForFinished();
//        netConnectReturnStr = QTextCodec::codecForName("gbk")->toUnicode(cmd.readAll());
//        bool b = netConnectReturnStr.contains("100% 丢失");
//        qDebug()<< b;
//        if(b){
//           emit(this->netConnectRecive());
//        }
//    }

    qDebug()<<"进入net线程run";
    while (!isInterruptionRequested())
    {
        testTimer = new QTimer;
        qDebug()<<"执行net线程run";
        testTimer->setInterval(this->intervalTime*1000);
        QObject::connect(testTimer,SIGNAL(timeout()),this,SLOT(net_connect()),Qt::DirectConnection);
        QTimer::singleShot(0, testTimer,SLOT(start()));
        //新的线程run里面一定要有exec的调用，否则无法接受timeout（）消息的。
        //exec();就是一个循环，
        exec();
        //直接调用start会报错，timer不能在另外线程中启动

    }
}
