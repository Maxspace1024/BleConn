#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QLowEnergyController>
#include <QBluetoothDeviceInfo>
#include <QBluetoothLocalDevice>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    QBluetoothDeviceDiscoveryAgent* agent;
    QLowEnergyController* bleCtrller;
    QBluetoothLocalDevice* locDev;
    QList<QBluetoothDeviceInfo> devicesList;
    QList<QLowEnergyService*> servicesList;
    QList<QLowEnergyCharacteristic> characList;

private slots:
    void on_startBtn_clicked();
    void on_stopBtn_clicked();
    void on_connectBtn_clicked();
    void on_disconnectBtn_clicked();
    void on_devNameLW_currentRowChanged(int currentRow);
    void on_servicesLW_currentRowChanged(int currentRow);
    void on_characLW_currentRowChanged(int currentRow);

    void on_agentDeviceDiscov(const QBluetoothDeviceInfo &info);
    void on_agentFin();
    void on_agentCancel();

    void on_ctrllerConnect();
    void on_ctrllerDisconnect();
    void on_ctrllerErrorOccur(QLowEnergyController::Error err);
    void on_ctrllerDiscoverFin();

    //void on_servCharacChange(const QLowEnergyCharacteristic &charac, const QByteArray &newVal);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
