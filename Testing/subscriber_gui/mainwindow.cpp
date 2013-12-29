#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <mosquittopp.h>


#define MQTT_SERVER "m10.cloudmqtt.com"
#define MQTT_PORT 10726
#define MQTT_USER "ymvhmlxw"
#define MQTT_PASS "Ind2uLCD4IJt"
#define MQTT_TOPIC "temp/device1"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_pSub(NULL)
{
    ui->setupUi(this);

    mosqpp::lib_init();

    m_pSub = new MySubscriber(ui->label, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS, MQTT_TOPIC);

    QObject::connect(&m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    m_timer.start(0);
}

MainWindow::~MainWindow()
{
    m_timer.stop();
    m_pSub->disconnect();
    delete m_pSub;

    mosqpp::lib_cleanup();
    delete ui;
}

void MainWindow::onTimer()
{
    int rc  = m_pSub->loop();
    if( rc )
    {
        QString text = "reconnecting on code " +QString::number(rc);
        ui->statusBar->showMessage(text);
        m_pSub->reconnect();
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}
