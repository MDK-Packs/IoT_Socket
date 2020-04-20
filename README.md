# IoT_Socket
Simple IP Socket (BSD like)

## Purpose
This repository contains two implementations of a simple BSD like IoT socket interface. The IoT Socket API is the glue logic
between an IoT cloud connector and the underlying network stack.

Currently, an implementation for
the [MDK-Middleware](https://www2.keil.com/mdk5/middleware) network stack or a
[CMSIS-Driver WiFi](https://arm-software.github.io/CMSIS_5/Driver/html/group__wifi__interface__gr.html) is available. This
makes it easy to retarget an IoT application running on an Arm Cortex-M based microcontroller device to either wired or
wireless IP connections. A generator script is available that builds a
[CMSIS-Pack](https://arm-software.github.io/CMSIS_5/Pack/html/index.html) that can be used in IDEs supporting the CMSIS-Pack
concept.


## Directory Structure

| Directory          | Content                                             |
|:-------------------|:----------------------------------------------------|
| documentation      | IoT Socket pack documentation                       |
| include            | Header file with the IoT Socket API                 |
| source             | Source files                                        |
| source/lwip        | Implementation for the lwIP network stack           |
| source/mdk_network | Implementation for the MDK-Middleware network stack |
| source/wifi        | Implementation for a WiFi CMSIS-Driver              |

## Build
To build the pack, run the [gen_pack.sh](gen_pack.sh) script.

## Other related GitHub repositories

| Repository                  | Description                                               |
|:--------------------------- |:--------------------------------------------------------- |
| [CMSIS](https://github.com/ARM-software/cmsis_5)                 | CMSIS                                                                             |
| [CMSIS-FreeRTOS](https://github.com/arm-software/CMSIS-FreeRTOS) | CMSIS-RTOS adoption of FreeRTOS                                                   |
| [CMSIS-Driver](https://github.com/arm-software/CMSIS-Driver)     | Generic MCU driver implementations and templates for Ethernet MAC/PHY and Flash   |
| [MDK-Packs](https://github.com/mdk-packs)                        | IoT cloud connectors as trail implementations for MDK (help us to make it generic)|

## License
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
