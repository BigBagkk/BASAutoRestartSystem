#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QMessageBox>
#include <QProcess> //使用CMD指令
#include <QtNetwork>
//FOR RS232
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "switchcontrol.h"
void outputMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    static QMutex mutex;
    mutex.lock();

    QString text;
    switch(type)
    {
    case QtDebugMsg:
        text = QString("Debug");
        break;

    case QtWarningMsg:
        text = QString("Warning");
        break;

    case QtCriticalMsg:
        text = QString("Critical");
        break;

    case QtFatalMsg:
        text = QString("Fatal");
        break;
    case QtInfoMsg:
        text = QString("Info");
        break;
    }

   // QString context_info = QString("File:(%1) Line:(%2)").arg(QString(context.file)).arg(context.line);
    QString current_date_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString current_date = QString("%1").arg(current_date_time);
    QString message = QString("%1|%2|%3").arg(current_date).arg(text).arg(msg);

    QFile file("log.txt");
    file.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream text_stream(&file);
    text_stream << message << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->ui_init();
    this->recordTime = 0;
    qDebug()<<"1111222";
    this->threadCom.changeTxState(false);
    this->threadCom.TxData.clear();
    //初始化日志程序11111111222223333
    qInstallMessageHandler(outputMessage);

    connect(&this->threadNet, SIGNAL(netConnectRecive()), this, SLOT(displayNetConnectReturn()));//显示网络访问结果
    connect(&this->threadNet, SIGNAL(netDisconnect()), this, SLOT(netDisconnect()));//
    connect(&this->threadNet, SIGNAL(netConnect()), this, SLOT(netConnect()));
    connect(&this->threadCom, SIGNAL(comRecive()), this, SLOT(displayRxData()));



    //设置开关状态
    pSwitchControl = new SwitchControl(ui->widget_switch);

    pSwitchControl->setCheckedColor(QColor(0, 160, 230));
    // 连接信号槽
    connect(pSwitchControl, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
    //pSwitchControl->resize(100,50);
    //SwitchControl *pGreenSwitchControl = new SwitchControl(ui->widget_switch);
    //SwitchControl *pDisabledSwitchControl = new SwitchControl(this);
    //pGreenSwitchControl->setToggle(true);
    //pGreenSwitchControl->setCheckedColor(QColor(0, 160, 230));
    // 设置状态、样式
    //pGreenSwitchControl->setToggle(true);
    //pGreenSwitchControl->setCheckedColor(QColor(0, 160, 230));
   // pDisabledSwitchControl->setDisabled(true);
   // pDisabledSwitchControl->setToggle(true);
//    qDebug("This is a debug message");
//    qWarning("This is a warning message");
//    qCritical("This is a critical message");
//    qFatal("This is a fatal message");
    //测试使用CMD指令ping访问IP
//    QProcess * cmd =new QProcess();
//    cmd->start("ping 192.168.1.1");
//    cmd->waitForReadyRead();
//    cmd->waitForFinished();
//    qDebug() << QString(cmd->readAll());

    //测试使用HTTP协议访问IP
//    QNetworkAccessManager * mgr = new QNetworkAccessManager(this);
//    QNetworkRequest url ;
//    url.setUrl(QUrl("http://192.168.0.1"));
//    mgr->get(url);
//    connect(mgr,SIGNAL(finished(QNetworkReply*)),this,SLOT(replyFinished(QNetworkReply*)));
}

MainWindow::~MainWindow()
{
    delete ui;
    //delete cmd;
}
void MainWindow::onToggled(bool bChecked)
{
    //qDebug() << "State : " << bChecked;
    if(bChecked){
        if(ui->lineEdit_IP->text()==""||ui->lineEdit_interval_time->text()==""||ui->lineEdit_disconnect_time->text()=="")
        {
            QMessageBox::information(this,"waring!!",tr("网络设置项不能为空！"));
            pSwitchControl->setToggle(false);
            return;
        }
        if(!threadCom.isRunning())
        {
            QMessageBox::information(this,"waring!!",tr("串口未打开"));
            pSwitchControl->setToggle(false);
            return;
        }
        bool ok;
        threadNet.netConnectStr=ui->lineEdit_IP->text();
        threadNet.intervalTime=ui->lineEdit_interval_time->text().toInt(&ok,10);
        if(!ok){
            QMessageBox::information(this,"w","访问间隔时间格式不正确！！");
            pSwitchControl->setToggle(false);
            return;
        }
        this->disconnectTime=ui->lineEdit_disconnect_time->text().toInt(&ok,10);
        if(!ok){
            QMessageBox::information(this,"w","访问间隔时间格式不正确！！");
            pSwitchControl->setToggle(false);
            return;
        }
        threadNet.stopped = false;
        threadNet.start();
        if(threadNet.isRunning()){
            ui->label_net_set_status->setText("运行中");
            ui->lineEdit_IP->setDisabled(true);
            ui->lineEdit_interval_time->setDisabled(true);
            ui->lineEdit_disconnect_time->setDisabled(true);
        }
    }
    else{
        //threadNet.stopped = true;
        threadNet.stopTimer();
        if(!threadNet.isRunning())
        {
            ui->label_net_set_status->setText("停止");
            ui->lineEdit_IP->setEnabled(true);
            ui->lineEdit_interval_time->setEnabled(true);
            ui->lineEdit_disconnect_time->setEnabled(true);
        }
    }
}
//接收到网络中断信号后
void MainWindow::netDisconnect()
{
    if(recordTime==0){
        recordTime=QDateTime::currentDateTime().toTime_t();
    }
    else
    {
        if(QDateTime::currentDateTime().toTime_t()-recordTime>disconnectTime)
        {
            //断电
            this->on_pushButton_YV1_close_clicked();
            recordTime = 0;//计时器重新计算
            //因为527断电，暂停网络监测
            threadNet.stopped = true;
            qDebug()<<"暂停网络监测";
            //断电后，等10秒再上电，开启定时器，等待527重新上电。
            QTimer::singleShot( 10000, this, SLOT(recoverTimeout()));
        }
    }
}
//接收到网络正常信号
void MainWindow::netConnect()
{
    recordTime = 0;
}
//等待527上电的定时器触发函数
void MainWindow::recoverTimeout()
{
    qDebug()<<"等待10秒后上电";
    //上电527
    this->on_pushButton_YV1_open_clicked();
    //上电后，再等待30秒，等待527初始化完成，再开启网络监测
    QTimer::singleShot( 30000, this, SLOT(recoverNetConnectTest()));
}
//上电后，再等待30秒，等待527初始化完成，再开启网络监测
void MainWindow::recoverNetConnectTest()
{
    qDebug()<<"等待30秒后恢复网络监测";
    //开启网络监测
    threadNet.stopped = false;
}
/*
 * 接收到串口数据后，触发函数
 */
void MainWindow::displayRxData()
{
//    QString str;
//    char tmp[100];
//    char *buf;
//    char var;
//    QDateTime *datatime=new QDateTime(QDateTime::currentDateTime());
//    if(threadCom.requestData.size()>0)
//   {
//        str="收到数据: ";
//        str+=datatime->time().toString();
//        ui->textEdit_reveive_data->append(str);
//        str.clear();

//        buf=threadCom.requestData.data();
//        qDebug() << "receive num:" <<threadCom.requestData.size();
//        for(var=0;var<threadCom.requestData.size();var++)
//        {
//            ::snprintf(tmp,100, "%02X", (unsigned char)(*buf));
//            buf++;
//            str+=QString::fromUtf8(tmp);
//            str+=" ";
//        }
//        ui->textEdit_reveive_data->append(str);
//    }
//    threadCom.requestData.clear();

    ui->textEdit_reveive_data->append(QString(threadCom.requestData));

    if(QString(threadCom.requestData).contains("OUT1 ON")){
        qDebug("527断电成功");
    }
    if(QString(threadCom.requestData).contains("OUT1 OFF")){
        qDebug("527上电成功");
    }
    threadCom.requestData.clear();
}
/*
 *  接收到网络访问结果后，触发信号，触发这个函数显示访问结果，并记录断网时间，
 */
void MainWindow::displayNetConnectReturn()
{
   // QMessageBox::information(this,tr("Tips"),tr("打开网络线程"),QMessageBox::Yes);
    ui->textEdit_reveive_data->append(threadNet.netConnectReturnStr);
}

void MainWindow::replyFinished(QNetworkReply *reply)
{
    QVariant statusCodeV = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    qDebug()<<"get return:"<<statusCodeV.toInt()<<endl;
    if (statusCodeV.toInt()==200){
        //QTextCodec *codec = QTextCodec::codecForName("utf8");

        //QString all = codec->toUnicode(reply->readAll());
        qDebug()<<"get return:"<<statusCodeV.toInt()<<endl;
        //qDebug()<<all<<endl;
        //ui->textEdit->setText(all);

    }else{
        qDebug()<<"get a error:"<<statusCodeV.toInt()<<endl;
    }
    reply->deleteLater();
}


/*
 *  下拉框加载现有COM口编号
 */
void MainWindow::ui_init(void)
{
    //前提：<pre name="code" class="plain">QT       += serialport
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if (serial.open(QIODevice::ReadWrite))
        {
           ui->comboBox_com_num->addItem(info.portName());
           serial.close();
        }
    }
}

void MainWindow::on_pushButton_open_com_clicked()
{
    //已选中串口号
    if(ui->comboBox_com_num->currentText()!="")
    {
        //start recive thread
        threadCom.startCom();
        qDebug() << "Brush:" <<"thread starting";
        threadCom.setPortnum(ui->comboBox_com_num->currentText());
        threadCom.setBuadRate(ui->comboBox_band->currentText());
        threadCom.changeComState(false);
        threadCom.start();
        ui->textEdit_tx->append("串口打开完成,串口号:"+ui->comboBox_com_num->currentText()+"波特率:"+ui->comboBox_band->currentText());
        //修改串口显示
        this->setWindowTitle("BAS自复位系统:串口打开:"+ui->comboBox_com_num->currentText()+","+ui->comboBox_band->currentText());

    }else
    {
        QMessageBox::information(this,tr("Tips"),tr("请选择串口号"),QMessageBox::Yes);
    }
}
void MainWindow::on_pushButton_close_com_clicked()
{
        qDebug() << "Brush:" <<"thread stoping";
        threadCom.stop();
        qDebug() << "Brush:" <<"com close";
        ui->textEdit_tx->append("串口关闭完成");
        this->setWindowTitle("BAS自复位系统:串口关闭");
}

void MainWindow::on_pushButton_send_clicked()
{
    if(threadCom.com_opened)
    {
            //Qstring使用unincode编码，每个字符占2个字节
            //QbyteArray则相当于char[]
            //unincode可以转换成UTF-8格式，转换后，英文字符部分与ASCII一样，占一个字节。
            //调用append函数 会自动将Unicode调用QString::toUtf8()后加入到Qbyteberry中。
            //因此，对于ASCII不需要将Qstring类型转换就可以直接将其append到QbyteArray中发送
            //MyThreadA->txdata.append(this->LineEdit_send->text().toUtf8().toHex());
            threadCom.TxData.clear();
            threadCom.TxData.append(ui->lineEdit_send_data->text());
            threadCom.changeTxState(true);
            ui->textEdit_tx->append(ui->lineEdit_send_data->text());
    }
    else
    {
        QMessageBox::information(this,"waring!!",tr("串口未打开！！"));
    }
}
/*
 *  电磁阀1 吸合
 */
void MainWindow::on_pushButton_YV1_close_clicked()
{
    qDebug("控制继电器1吸合：527断电");
    this->sendCOM("A11T");
}
/*
 *  电磁阀1 释放
 */
void MainWindow::on_pushButton_YV1_open_clicked()
{
    qDebug("控制继电器1释放：527上电");
    this->sendCOM("A10T");
}
/*
 *  电磁阀2 吸合
 */
void MainWindow::on_pushButton_YV2_close_clicked()
{
    this->sendCOM("A21T");
}

void MainWindow::on_pushButton_YV2_open_clicked()
{
    this->sendCOM("A20T");
}
/*
 * 内部函数：向串口发送字符串
 */
void MainWindow::sendCOM(QString message)
{
    if(threadCom.com_opened)
    {
            //Qstring使用unincode编码，每个字符占2个字节
            //QbyteArray则相当于char[]
            //unincode可以转换成UTF-8格式，转换后，英文字符部分与ASCII一样，占一个字节。
            //调用append函数 会自动将Unicode调用QString::toUtf8()后加入到Qbyteberry中。
            //因此，对于ASCII不需要将Qstring类型转换就可以直接将其append到QbyteArray中发送
            //MyThreadA->txdata.append(this->LineEdit_send->text().toUtf8().toHex());
            threadCom.TxData.clear();
            threadCom.TxData.append(message);
            threadCom.changeTxState(true);
            ui->textEdit_tx->append(message);
    }
    else
    {
        QMessageBox::information(this,"waring!!",tr("串口未打开！！"));
    }
}
/*
 *  测试网络连接按键
 */
void MainWindow::on_pushButton_testConnect_clicked()
{
    if(ui->lineEdit_IP->text()=="")
    {
        QMessageBox::information(this,"waring!!",tr("IP不能为空"));
        return;
    }
    threadNet.netConnectStr=ui->lineEdit_IP->text();
    threadNet.stopped = false;
    if(threadNet.isRunning()){
        threadNet.startTimer();
    }
    else
    {
        threadNet.start();
    }

//    return;
//    //测试使用CMD指令ping访问IP
//    cmd =new QProcess();
//    cmd->start("ping "+ui->lineEdit_IP->text());
//    cmd->waitForReadyRead();
//    cmd->waitForFinished();
//    //qDebug() << QString(cmd->readAll());

//    QTextCodec * codec = QTextCodec::codecForName("gbk");
//    //webContent = codec->toUnicode(reply->readAll());
//    //ui->textEdit_reveive_data->append(QString(cmd->readAll()));
//    ui->textEdit_reveive_data->append(codec->toUnicode(cmd->readAll()));

}

void MainWindow::on_pushButton_stop_clicked()
{
    //threadNet.stopped = true;
    threadNet.stopTimer();
    if(!threadNet.isRunning())
    {
        ui->lineEdit_IP->setEnabled(true);
        ui->lineEdit_interval_time->setEnabled(true);
        ui->lineEdit_disconnect_time->setEnabled(true);
    }
}



void MainWindow::on_pushButton_start_clicked()
{
    if(ui->lineEdit_IP->text()==""||ui->lineEdit_interval_time->text()==""||ui->lineEdit_disconnect_time->text()=="")
    {
        QMessageBox::information(this,"waring!!",tr("网络设置项不能为空！"));
        return;
    }
    if(!threadCom.isRunning())
    {
        QMessageBox::information(this,"waring!!",tr("串口未打开"));
        return;
    }
    bool ok;
    threadNet.netConnectStr=ui->lineEdit_IP->text();
    threadNet.intervalTime=ui->lineEdit_interval_time->text().toInt(&ok,10);
    if(!ok){
        QMessageBox::information(this,"w","访问间隔时间格式不正确！！");
        return;
    }
    this->disconnectTime=ui->lineEdit_disconnect_time->text().toInt(&ok,10);
    if(!ok){
        QMessageBox::information(this,"w","访问间隔时间格式不正确！！");
        return;
    }
    //threadNet.stopped = false;
    threadNet.start();
    if(threadNet.isRunning()){
        ui->lineEdit_IP->setDisabled(true);
        ui->lineEdit_interval_time->setDisabled(true);
        ui->lineEdit_disconnect_time->setDisabled(true);
    }
}
