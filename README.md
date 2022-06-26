# BleConn
* 用於練習與測試Qt Bluetooth Low Energy 的連線情況

![image](https://github.com/Maxspace1024/BleConn/blob/master/img/mainpage.png)

## 20220626
* 上傳source code
  * serv->readCharacteristic(c)可以取得Characteristic的新value
  * serv->discoverDetail()用來尋找Servie中的Characteristic
    * serv->state()檢查搜尋狀況
    * `ServiceState::ServiceDiscovered`已經捨棄
    * 要使用`ServiceState::RemoteServiceDiscovered`
    * 在狀態下`ServiceState::ServiceDiscovering`中斷controller連線會導致Exception
* 模組化或是給定UUID就可以取得資料
* 要增加Write Characteristic的功能
  * 在QLineEdit輸入Hex Code並轉換為`QByteArray`
  * 新增Write Button
```C
void writeCharacteristic(
    const QLowEnergyCharacteristic &characteristic, 
    const QByteArray &newValue, 
    QLowEnergyService::WriteMode mode = WriteWithResponse
)
```
* `QLowEnergyService`有以下`SIGNAL`
  * characteristicChanged
    * Indicate或Notify可能會用到這個
  * characteristicRead
  * characteristicWritten
  * descriptorRead
  * descriptorWritten
  * error
  * stateChanged
* Descripter是什麼東西???
* 解決Characteristic properties沒有QFlag::read而產生的例外