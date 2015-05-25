#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QFile>
#include <QTimer>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void onUdpCmdRead();
    void onUdpUpdateRead();
    void onTimer();

    void on_pushButton_file_clicked();

    void on_pushButton_send_clicked();

private:
    Ui::Widget *ui;
    QUdpSocket *s;
    void oneSend();
    QFile file;
    QUdpSocket *s_update;
    char sendBuf[1024+4+2];
    qint32 sendIndex;
    qint16 readLength;
    QTimer *timer;
    int errorCnt = 0;
    QHostAddress host;
};

#endif // WIDGET_H
