2 # STM32F103蓝牙&传感器

F103ZE_Pro开发板

## 连接方式说明：

### FSR压力传感器

VCC < ------------ > 5V

GND < ------------ > GND

AO< ------------ > PA1


### ATK-IMU901 模块

VCC < ------------ > 3.3V 

GND < ------------ > GND

TXD < ------------ > PA3

RXD < ------------ >  PA2


### ATK-BLE01 模块

VCC < ------------ > 5V

GND < ------------ > GND

TXD < ------------ > PB11

RXD < ------------ > PB10

STA < ------------ > PA4

WKUP < ----------- > PA15

### 注释

压力传感器量程设置 main.c line:29-32

IMU901加速度传感器量程设置 imu901.c line:367 ref:传感器手册 sec 6.2.6