# Introduction

The IoT Socket is a [BSD like](https://en.wikipedia.org/wiki/Berkeley_sockets) IP socket that implements the interface between IoT cloud connectors (IoT clients) and the underlying communication stack as shown on the picture below.

![Structure of an IoT application](./iot_block_diagram.png)

This [CMSIS Pack](https://www.open-cmsis-pack.org/) supports  AC6, GCC IAR, and LLVM compiler.

## Supported network stacks

IoT Socket implementation variants are available for the following network stacks:
- [MDK-Middleware Network](https://arm-software.github.io/MDK-Middleware/latest/Network/index.html)
- [FreeRTOS-Plus-TCP](https://www.freertos.org/Documentation/03-Libraries/02-FreeRTOS-plus/02-FreeRTOS-plus-TCP/01-FreeRTOS-Plus-TCP)
- [lwIP](https://en.wikipedia.org/wiki/LwIP)
- [CMSIS-Driver WiFi](https://arm-software.github.io/CMSIS_6/latest/Driver/group__wifi__interface__gr.html)
- [VSocket](https://arm-software.github.io/AVH/main/simulation/html/group__arm__vsocket.html) for [Arm Virtual Hardware](https://www.arm.com/products/development-tools/simulation/virtual-hardware)

Using the **IoT Socket Multiplexer** allows to switch network communication stacks at run-time, for example from a wireless to wired connection.

## Example

The [AWS MQTT Mutual Authentication Demo project](https://github.com/Arm-Examples/AWS_MQTT_MutualAuth_Demo) provides an example how the software is used in a real-life project.

## Links

- [Documentation](https://mdk-packs.github.io/IoT_Socket/latest/index.html)
- [Repository](https://github.com/MDK-Packs/IoT_Socket)
- [Issues](https://github.com/MDK-Packs/IoT_Socket/issues)
