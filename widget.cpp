#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>
#include <QTime>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    s = new QUdpSocket;
    s->bind(8001);
    connect(s,SIGNAL(readyRead()),this,SLOT(onUdpCmdRead()));
    s_update = new QUdpSocket();
    s_update->bind(9001,QUdpSocket::ShareAddress);
    connect(s_update,SIGNAL(readyRead()),this,SLOT(onUdpUpdateRead()));
    timer = new QTimer();
    timer->setInterval(3000);
    timer->setSingleShot(true);
    connect(timer,SIGNAL(timeout()),this,SLOT(onTimer()));
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    while(ui->comboBox->count() > 0)
        ui->comboBox->removeItem(0);
    QByteArray cmd;
    cmd.append((char)0x01);
    cmd.append((char)0x00);
    s->writeDatagram(cmd,QHostAddress("192.168.1.255"),8000);
}

void Widget::onUdpCmdRead()
{
    while (s->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(s->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;
            QByteArray answer;

            s->readDatagram(datagram.data(), datagram.size(),
                                    &sender, &senderPort);

            qDebug()<<datagram.toHex();
            char* p = datagram.data();
            switch (p[0]) {
            case (char)0x01:
                ui->comboBox->insertItem(ui->comboBox->count(),
                                         sender.toString() + ":version " + QString::number(p[2]) + "."+QString::number(p[3]));
                break;
            default:
                break;
            }
    }

}

void Widget::onUdpUpdateRead()
{
    QTime time;
    while (s_update->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(s_update->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            s_update->readDatagram(datagram.data(), datagram.size(),
                                    &sender, &senderPort);

            qDebug()<<datagram.toHex()<<sender.toString()<<senderPort;
            char *dat = datagram.data();
            int index = *((int*)dat);
            short status = *((short*)(dat + 4));
            int m_lastIndex;
            if(datagram.length() == 10)
            {
                m_lastIndex = *((int*)(dat + 6));
                qDebug()<<index<<m_lastIndex;
            }

            if(index == sendIndex
                    && status == (short)0x0000)
            {
                sendIndex ++;
                timer->stop();
                errorCnt = 0;
                ui->textBrowser->append("reply of " + QString::number(index));
                readLength = file.read(sendBuf + 6,1024);
            }
            else if(index == sendIndex
                    && status == (short)0x0002)
            {
                timer->stop();
                errorCnt = 0;
                ui->textBrowser->append("reply of lastIndex, tranform complete");
                file.close();
                return;
            }
            else
                ui->textBrowser->append("error of " + QString::number(sendIndex) + ", retry");
            time = QTime::currentTime().addMSecs(200);
            /*while(QTime::currentTime() < time)
            {
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }*/
            oneSend();
        }
}

void Widget::onTimer()
{
    //qDebug()<<"onTimer";
    errorCnt ++;
    if(errorCnt < 10)
        oneSend();
    else
    {
        ui->textBrowser->append("timeOut 10 times, stopped");
        file.close();
    }
}


void Widget::oneSend()
{
    *((qint32*)sendBuf) = sendIndex;
    *((qint16*)(sendBuf + 4)) = readLength;
    ui->textBrowser->append("Send Data:  index = " + QString::number(sendIndex) + "  len = " + QString::number(readLength));
    s_update->writeDatagram(sendBuf,readLength + 6, host,9000);
    timer->start();
}


void Widget::on_pushButton_file_clicked()
{
    file.setFileName(QFileDialog::getOpenFileName());
    ui->lineEdit->setText(file.fileName());
    ui->textBrowser->append("Set Send File:  " + file.fileName());
    sendIndex = 0;
    host = QHostAddress(ui->comboBox->currentText().split(':').at(0));
}

void Widget::on_pushButton_send_clicked()
{
    file.open(QFile::ReadOnly);
    errorCnt = 0;
    readLength = file.read(sendBuf + 6,1024);
    oneSend();
}
