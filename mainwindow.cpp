#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    locDev = new QBluetoothLocalDevice(this);
    agent = new QBluetoothDeviceDiscoveryAgent(this);
    agent->setLowEnergyDiscoveryTimeout(10000);

    connect(agent,
            SIGNAL(deviceDiscovered(const QBluetoothDeviceInfo &)),
            this,
            SLOT(on_agentDeviceDiscov(const QBluetoothDeviceInfo &)));
    connect(agent,
            SIGNAL(finished()),
            this,
            SLOT(on_agentFin()));
    connect(agent,
            SIGNAL(canceled()),
            this,
            SLOT(on_agentCancel()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startBtn_clicked()
{
    ui->devNameLW->clear();
    ui->servicesLW->clear();
    ui->characLW->clear();
    ui->characInfoLabel->setText(tr(""));
    ui->statusLabel->setText(tr("start"));
    devicesList.clear();
    servicesList.clear();
    characList.clear();

    ui->stopBtn->setEnabled(true);
    ui->startBtn->setEnabled(false);

    agent->start();
}
void MainWindow::on_stopBtn_clicked()
{
    agent->stop();
}
void MainWindow::on_connectBtn_clicked()
{
    ui->connectBtn->setEnabled(false);
    ui->disconnectBtn->setEnabled(true);

    int row = ui->devNameLW->currentRow();
    if(row>=0)
    {
        QBluetoothDeviceInfo remoteDevice = devicesList.at(row);
        bleCtrller = QLowEnergyController::createCentral(remoteDevice,locDev->address(),this);
        bleCtrller->connectToDevice();

        connect(bleCtrller,
                SIGNAL(connected()),
                this,
                SLOT(on_ctrllerConnect()));
        connect(bleCtrller,
                SIGNAL(disconnected()),
                this,
                SLOT(on_ctrllerDisconnect()));
        connect(bleCtrller,
                SIGNAL(errorOccurred(QLowEnergyController::Error)),
                this,
                SLOT(on_ctrllerErrorOccur(QLowEnergyController::Error)));
        connect(bleCtrller,
                SIGNAL(discoveryFinished()),
                this,
                SLOT(on_ctrllerDiscoverFin()));
    }
    else{
        ui->statusLabel->setText(tr("no item selected!!"));
    }
}
void MainWindow::on_disconnectBtn_clicked()
{
    ui->connectBtn->setEnabled(true);
    ui->disconnectBtn->setEnabled(false);

    if(bleCtrller->state() != QLowEnergyController::UnconnectedState)
    {
        //刪除動態分配之記憶體 : QLowEnergyService*
        for(auto &s : servicesList)
            delete s;
        servicesList.clear();

        bleCtrller->disconnectFromDevice();
        delete bleCtrller;
    }
}

void MainWindow::on_agentDeviceDiscov(const QBluetoothDeviceInfo& info)
{
    ui->devNameLW->addItem( info.name());
}
void MainWindow::on_agentFin()
{
    devicesList = agent->discoveredDevices();
    ui->statusLabel->setText(tr("finished"));

    ui->stopBtn->setEnabled(false);
    ui->startBtn->setEnabled(true);
    ui->connectBtn->setEnabled(true);
}
void MainWindow::on_agentCancel()
{
    devicesList = agent->discoveredDevices();
    ui->statusLabel->setText(tr("canceled/stopped"));

    ui->stopBtn->setEnabled(false);
    ui->startBtn->setEnabled(true);
    ui->connectBtn->setEnabled(true);
}

void MainWindow::on_devNameLW_currentRowChanged(int currentRow)
{
    //若還在掃瞄周圍，選取item無效
    if(!agent->isActive())
    {
        //index都在list的範圍內
        if(0 <= currentRow && currentRow < devicesList.size())
        {
            QBluetoothDeviceInfo info = devicesList.at(currentRow);
            ui->uuidLabel->setText(info.address().toString());
        }
        else
        {
            ui->statusLabel->setText(tr("out of range!!"));
        }
    }
    else{
        ui->statusLabel->setText(tr("agent is scanning!!"));
    }
}
void MainWindow::on_servicesLW_currentRowChanged(int currentRow)
{
    //index都在list的範圍內
    if(0 <= currentRow && currentRow < servicesList.size())
    {
        //qDebug() << "servicesLW:" << currentRow;
        QLowEnergyService* serv = servicesList.at(currentRow);

        //檢查service是否搜尋完畢
        if(serv->state() == QLowEnergyService::RemoteServiceDiscovered)
        {
            ui->characLW->clear();
            characList = serv->characteristics();
            for(auto &c : characList)
                ui->characLW->addItem(c.uuid().toString());
        }
        else
            qDebug() << "serv discover not yet";

    }
    else
        ui->statusLabel->setText(tr("out of range!! servLW"));
}
void MainWindow::on_characLW_currentRowChanged(int currentRow)
{
    int sRow = ui->servicesLW->currentRow();
    if(0 <= sRow && sRow < servicesList.size())
    {
        QLowEnergyService* serv = servicesList.at(sRow);

        if(0 <= currentRow && currentRow < characList.size())
        {
            QLowEnergyCharacteristic c = characList.at(currentRow);
            serv->readCharacteristic(c);
            {
                QString prop = "";
                if(QLowEnergyCharacteristic::Unknown & c.properties())
                        prop += "UnKnown|";
                if(QLowEnergyCharacteristic::Broadcasting & c.properties())
                        prop += "Broadcasting|";
                if(QLowEnergyCharacteristic::WriteNoResponse & c.properties())
                        prop += "WriteNoResponse|";
                if(QLowEnergyCharacteristic::Write & c.properties())
                        prop += "Write|";
                if(QLowEnergyCharacteristic::Read & c.properties())
                        prop += "Read|";
                if(QLowEnergyCharacteristic::Notify & c.properties())
                        prop += "Notify|";
                if(QLowEnergyCharacteristic::Indicate & c.properties())
                        prop += "Indicate|";
                if(QLowEnergyCharacteristic::WriteSigned & c.properties())
                        prop += "WriteSigned|";
                if(QLowEnergyCharacteristic::ExtendedProperty & c.properties())
                        prop += "ExtendedProperty|";

                QString characInfoStr = QString(
                    "uuid:\t%1\n"
                    "value:\t%2\n"
                    "proper:\t%3\n"
                ).arg(c.uuid().toString(),c.value().toHex(' '),prop);

                ui->characInfoLabel->setText(characInfoStr);
            }
        }
        else
            ui->statusLabel->setText(tr("out of range Charac!!"));
    }
    else
        ui->statusLabel->setText(tr("out of range!! servLW2"));
}

void MainWindow::on_ctrllerConnect()
{
    ui->startBtn->setEnabled(false);

    ui->statusLabel->setText(tr("connected"));

    ui->servicesLW->clear();

    bleCtrller->discoverServices();
}
void MainWindow::on_ctrllerDisconnect()
{
    ui->startBtn->setEnabled(true);
    ui->statusLabel->setText(tr("disconnected"));
}
void MainWindow::on_ctrllerErrorOccur(QLowEnergyController::Error err)
{
    switch(err)
    {
    case QLowEnergyController::NoError:
        qDebug() << "NoError";
        break;
    case QLowEnergyController::UnknownError:
        qDebug() << "UnknownError";
        break;
    case QLowEnergyController::UnknownRemoteDeviceError:
        qDebug() << "UnknownRemoteDeviceError";
        break;
    case QLowEnergyController::NetworkError:
        qDebug() << "NetworkError";
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        qDebug() << "InvalidBluetoothAdapterError";
        break;
    case QLowEnergyController::ConnectionError:
        qDebug() << "ConnectionError";
        break;
    case QLowEnergyController::AdvertisingError:
        qDebug() << "AdvertisingError";
        break;
    case QLowEnergyController::RemoteHostClosedError:
        qDebug() << "RemoteHostClosedError";
        break;
    case QLowEnergyController::AuthorizationError:
        qDebug() << "AuthorizationError";
        break;
    default:
        qDebug() << "default:? ctrller";
        break;
    }
}
void MainWindow::on_ctrllerDiscoverFin()
{
    QList<QBluetoothUuid> uuidsList = bleCtrller->services();
    for(auto &u : uuidsList)
    {
        QLowEnergyService* serv = bleCtrller->createServiceObject(u,this);
        servicesList.append(serv);
        ui->servicesLW->addItem(serv->serviceUuid().toString());

        serv->discoverDetails();
        /*
        connect(serv,
                SIGNAL(characteristicChanged(const QLowEnergyCharacteristic &, const QByteArray &)),
                this,
                SLOT(on_servCharacChange(const QLowEnergyCharacteristic &, const QByteArray &)));
        */
    }
}

/*
void MainWindow::on_servCharacChange(const QLowEnergyCharacteristic &charac, const QByteArray &newVal)
{
    qDebug() << charac.uuid() << " :: " << newVal.toHex(' ');
}
*/
