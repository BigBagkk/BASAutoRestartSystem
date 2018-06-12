#ifndef NETTHREAD
#define NETTHREAD

#include <QThread>
#include <QString>
#include <QProcess> //使用CMD指令
#include <QtNetwork>
#include <QTime>
class NetThread : public QThread
{
    Q_OBJECT
public:
    NetThread();
    ~NetThread();
    QString netConnectStr;
    QString netConnectReturnStr;
    int intervalTime;//访问间隔时间（秒）
    //int disconnectTime;//判定断网时间（秒）
    volatile bool stopped;
    void stopTimer();
    void startTimer();

signals:
    void netConnectRecive();
    void netDisconnect();
    void netConnect();
public:
    //QTimer * testTimer =NULL;
private slots:
    void net_connect();
protected:
    void run();


};
#endif // NETTHREAD

