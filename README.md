# Wi-Fi Middleware Core Library
This repo comprises core components needed for Wi-Fi connectivity support. The library bundles FreeRTOS, LwIP TCP/IP stack, and Mbed TLS for security, Cypress Wi-Fi host driver (WHD), Cypress secure sockets interface, configuration files, and associated code to bind these components together.

The ModusToolbox™ Wi-Fi code examples download this library automatically, so you don't need to.

## Features and Functionality
The following components are part of this library. These components are bundled as *.lib* entries; each *.lib* entry points to the respective repositories where they are hosted. All except LwIP and Mbed TLS libraries are hosted on Cypress' GitHub repository. LwIP and Mbed TLS libraries are hosted on their respective external repositories.

- **Cypress Wi-Fi Host Driver(WHD)** - Embedded Wi-Fi Host Driver that provides a set of APIs to interact with Cypress WLAN chips. Refer to [Cypress Wi-Fi Host Driver(WHD)](https://github.com/cypresssemiconductorco/wifi-host-driver) for details.

- **FreeRTOS** - FreeRTOS kernel, distributed as standard C source files with configuration header file, for use with PSoC 6 MCU. This FreeRTOS library is based on publicly available FreeRTOS library version 10.0.1.37. See [FreeRTOS](https://github.com/cypresssemiconductorco/freertos) web site for details.

- **CLib FreeRTOS Support Library** - This library provides the necessary hooks to make C library functions such as malloc and free thread safe. This implementation is specific to FreeRTOS; this library is required for building your application. See the [CLib FreeRTOS Support Library](https://github.com/cypresssemiconductorco/clib-support) web site for details.

- **LwIP** - A Lightweight open-source TCP/IP stack, version: 2.1.2. See the [LwIP](https://savannah.nongnu.org/projects/lwip/) web site for details.

   **Note**: Using this library in a project will cause LwIP to be downloaded on your computer. It is your responsibility to understand and accept the LwIP license.

- **MbedTLS** - An open source, portable, easy-to-use, readable and flexible SSL library that has cryptographic capabilities, version: 2.16.6. See the [MbedTLS](https://tls.mbed.org/) web site for details.

   **Note**: Using this library in a project will cause Mbed TLS to be downloaded on your computer. It is your responsibility to understand and accept the Mbed TLS license and regional use restrictions (including abiding by all applicable export control laws).

- **Cypress RTOS Abstraction Layer** - The RTOS Abstraction APIs allow middleware to be written to be RTOS-aware, but without depending on any particular RTOS. See the Cypress RTOS Abstraction Layer](https://github.com/cypresssemiconductorco/abstraction-rtos) repository for details.

- **Cypress Secure Sockets** - Network abstraction APIs for the underlying LwIP network stack and Mbed TLS security library. The Secure Sockets Library eases application development by exposing a socket-like interface for both secure and non-secure socket communication. See the [Cypress Secure Sockets](https://github.com/cypresssemiconductorco/secure-sockets) repository for details.

- **Predefined configuration files** - For FreeRTOS, LwIP and MbedTLS for typical embedded IoT use cases. See **Integration Notes** section for details.

- **Associated Glue Layer Between LwIP and WHD**

This library is designed to work with Cypress' PSoC kits with Wi-Fi capability, supported through ModusToolbox software. 

## Supported Platforms
This library and its features are supported on the following Cypress platforms:

- [PSoC 6 Wi-Fi-BT Prototyping Kit (CY8CPROTO-062-4343W)](https://www.cypress.com/documentation/development-kitsboards/psoc-6-wi-fi-bt-prototyping-kit-cy8cproto-062-4343w)

- [PSoC 62S2 Wi-Fi BT Pioneer Kit (CY8CKIT-062S2-43012)](https://www.cypress.com/documentation/development-kitsboards/psoc-62s2-wi-fi-bt-pioneer-kit-cy8ckit-062s2-43012)

## Quick Start
A set of pre-defined configuration files have been bundled with this library for FreeRTOS, LwIP, and Mbed TLS. These files are located in the *configs* folder. 

You should do the following: 

1. Copy the *FreeRTOSConfig.h* file from *configs* directory to the top-level code example directory in the project.

2. Configure the `MBEDTLS_USER_CONFIG_FILE` C macro to *configs/mbedtls_user_config.h* in the Makefile to provide the user configuration to the Mbed TLS library. The Makefile entry would look like as follows:

   ```
   DEFINES+=MBEDTLS_USER_CONFIG_FILE='"configs/mbedtls_user_config.h"'
   ```
3. Add the `CYBSP_WIFI_CAPABLE` build configuration to enable Wi-Fi functionality. The Makefile entry would look like as follows:

   ```
   DEFINES+=CYBSP_WIFI_CAPABLE
   ```
   - The LwIP configuration file (*./configs/lwipopts.h*) will be automatically picked up during compilation. Do not make any changes to the default *lwipopts.h* file. However, if any configurations changes are required, copy over the *lwipopts.h* file to the root folder of the code example before making the changes.

   - Secure Sockets, LwIP, and Mbed TLS libraries contain reference and test applications. To ensure that these applications do not conflict with the code examples, a *.cyignore* file is also included with this library.

4. Add the following to COMPONENTS in the code example project's Makefile - `FREERTOS`, `PSOC6HAL`, `LWIP`, `MBEDTLS`, and either `4343W` or `43012` depending on the platform. 

   For example, if your target is CY8CKIT-062S2-43012, the Makefile entry would look like as follows:

   ```
   COMPONENTS=FREERTOS PSOC6HAL LWIP MBEDTLS 43012
   ```

## Additional Information
* [Wi-Fi Middleware Core RELEASE.md](./RELEASE.md)

* [Wi-Fi Middleware Core API Reference Guide](https://cypresssemiconductorco.github.io/wifi-mw-core/api_reference_manual/html/index.html)

* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)

* [LwIP](https://savannah.nongnu.org/projects/lwip/)

* [MbedTLS](https://tls.mbed.org/)

* [Wi-Fi middleware core version](./version.txt)
