#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include "subscriber.h"

namespace Ui {
class MainWindow;
}

class MySubscriber : public Subscriber
{
public:
    MySubscriber(QLabel* pLabel, const char* host, int port,
                 const char* user, const char* pass, const char* topic)
        : Subscriber(NULL, host, port, user, pass, topic),
          m_pLabel(pLabel)
    {}

    void on_message(const struct mosquitto_message *message)
    {
        int value = *(int*)message->payload;
        m_pLabel->setText(QString(message->topic) + QString(" ")
                          + QString::number(message->mid) + QString(" ")
                          + QString::number(value));
    }

private:
    QLabel* m_pLabel;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onTimer();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    MySubscriber* m_pSub;
    QTimer m_timer;
};

#endif // MAINWINDOW_H
