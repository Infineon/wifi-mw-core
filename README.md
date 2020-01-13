# WiFi middleware core library
This repo comprises core components needed for WiFi connectivity support. The library bundles FreeRTOS, lwIP TCP/IP stack, mbedTLS for security, Cypress WiFi host driver (WHD), configuration files and associated code to bind these components together.

This library provides application developers a suite of libraries along with the necessary glue logic enabling WiFi secure network capability. 

The ModusToolbox WiFi code examples download this library automatically, so you don't need to.

## Features and functionality
The following components are part of this library. These components are bundled as ".lib" entries; each ".lib" points to the respective repositories, where they are hosted. All but lwIP and mbedTLS libraries are hosted on Cypress' Github repository. lwIP and mbedTLS libraries are hosted on their respective external repositories.

* Cypress Wifi Host Driver(WHD) - Embedded Wi-Fi Host Driver that provides a set of APIs to interact with Cypress WLAN chips
* FreeRTOS - FreeRTOS kernel, distributed as standard C source files with configuration header file, for use with the PSoC 6 MCU
* lwIP - A Lightweight open-source TCP/IP stack. Refer to [lwIP](https://savannah.nongnu.org/projects/lwip/) for details
   * NOTE: Using this library in a project will cause lwIP to be downloaded on your computer. It is your responsibility to understand and accept the lwIP license
* mbedTLS - An open source, portable, easy to use, readable and flexible SSL library, that has cryptographic capabilities. Refer to [mbedTLS](https://tls.mbed.org/) for details
   * NOTE: Using this library in a project will cause mbedTLS to be downloaded on your computer. It is your responsibility to understand and accept the Mbed TLS license and regional use restrictions (including abiding by all applicable export control laws).
* Cypress RTOS Abstraction Layer - The RTOS Abstraction APIs allow middleware to be written to be RTOS aware, but without depending on any particular RTOS
* Predefined configuration files for FreeRTOS, lwIP and mbedTLS for typical embedded IoT usecases. See Integration Notes section for details
* Associated glue layer between lwIP and WHD, and mbedTLS and lwIP

This library is designed to work with Cypress' PSOC kits with WiFi capability, supported through ModusToolbox software environment

## Supported platforms
This library and it's features are supported on following Cypress platforms:
* [PSoC6 WiFi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)
* CY8CKIT-062S2-43012

## Integration Notes
* A set of pre-defined configuration files have been bundled with this library for FreeRTOS, lwIP and mbedTLS. The developer is expected to: 
   * Copy the FreeRTOSConfig.h file from configs directory to the top level code example directory in the project
   * Configure C Macro MBEDTLS_USER_CONFIG_FILE to "configs/mbedtls_user_config.h". Add it to DEFINES in the code example project's Makefile. Also, add CYBSP_WIFI_CAPABLE to enable WiFi functionality. The Makefile entry would look like<br/>DEFINES=MBEDTLS_USER_CONFIG_FILE='"configs/mbedtls_user_config.h"' CYBSP_WIFI_CAPABLE
   * lwIP configuration file will be automatically picked up during compilation
* Libraries lwIP and mbedTLS contain reference and test applications. In order for these applications to not conflict with the code exampples, a cyignore file is also included with this library. This file has to be renamed to .cyignore and copied to the top level code example directory
* Add the following to COMPONENTS in the code example project's Makefile - FREERTOS, PSOC6HAL and either 4343W or 43012 depending on the platform. For instance, if CY8CKIT-062S2-43012 is chosen, then the Makefile entry would look like<br/>COMPONENTS=FREERTOS PSOC6HAL 43012

## Additional Information
* [WiFi middleware core RELEASE.md](./RELEASE.md)
* [WiFi middleware core API reference guide](https://cypresssemiconductorco.github.io/wifi-mw-core/api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [lwIP](https://savannah.nongnu.org/projects/lwip/)
* [mbedTLS](https://tls.mbed.org/)
* [WiFi middleware core version](./version.txt)
