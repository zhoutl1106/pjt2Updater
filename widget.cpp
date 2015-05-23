#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    s = new QUdpSocket;
    s->bind(8001);
    connect(s,SIGNAL(readyRead()),this,SLOT(onUdpRead()));
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

void Widget::onUdpRead()
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
                qDebug()<<"cmd1"<<p[0]<<p[1]<<p[2]<<p[3];
                ui->comboBox->insertItem(ui->comboBox->count(),
                                         sender.toString() + ":version " + QString::number(p[2]) + "."+QString::number(p[3]));
                break;
            default:
                break;
            }
    }

}
