## 连接方式说明：
[模块] < ------------ > [STM32F103开发板]

### FSR压力传感器（线性转压）

VCC < ------------ > 5V

GND < ------------ > GND

AO（模块1） < ------------ > PA1（ADC1 channel 1）

AO（模块2） < ------------ > PC0（ADC1 channel 10）

AO（模块3） < ------------ > PC1（ADC1 channel 11）


### ATK-IMU901 模块

VCC < ------------ > 3.3V 

GND < ------------ > GND

TXD < ------------ > PA3

RXD < ------------ > PA2


### ATK-BLE01 模块

VCC < ------------ > 5V

GND < ------------ > GND

TXD < ------------ > PB11

RXD < ------------ > PB10

STA < ------------ > PA4

WKUP < ----------- > PA15


### MPU-9250九轴传感器（预留）

VCC < ------------ > 3.3V

GND < ------------ > GND

SCL < ------------ > PB6

SDA < ------------ > PB7


## 连接示意图

![pin_connect](./PinConnections.svg) 


## PCB库文件（如需要，使用Altium Designer软件打开）

[ATK-BLE01](./pcblibs/ATK-BLE01_V1.9.PcbLib)

[ATK-IMU901](./pcblibs/ATK-IMU901.PcbLib)

[STM32F103ZE](./pcblibs/STM32F103ZE.PcbLib)

## 参考资料

[ATK-BLE01](http://www.openedv.com/docs/modules/iot/ATK-BLE01.html)

[ATK-IMU901](http://47.111.11.73/docs/modules/other/ATK-IMU901.html)

[STM32](https://pan.baidu.com/s/19XI8DBlvDGqzCfeJAvJaZA) (提取码：7fuc)

## 注：（除连接示意模块外还应包括）（见[原理图](./docs/STM32F103ZE核心板--原理图-1807M.pdf)）

- 最小系统和USB串口烧写电路，程序使用了ADC，因此ADC相关引脚也要正确供电

- RST复位电路

- BOOT模式选择(默认接地)



