# Cypress WiFi middleware core library

## What's Included?
Refer to the [README.md](./README.md) for a complete description of the WiFi middleware core

## Known Issues
| Problem | Workaround |
| ------- | ---------- |
| UDP and DTLS are not supported in the WiFi middleware core library | No workaround. Support will be added in a future release |
| With older versions of psoc6make earlier than 1.2.0, due to a mismatch in the target kit naming between ModusToolbox and WHD, results in NVRAM files not getting picked up | Rename the target directory names in Wifi_Host_Driver/resources/nvram from TARGET_CY8CPROTO_062_4343W to TARGET_CY8CPROTO-062-4343W (similarly for CY8CKIT-062S2-43012). The latest psoc6make library (1.2.0 onwards) handles this issue automatically and avoids the need for this workaround |

## Changelog
### v1.0.0
* Initial release for WiFi middleware core
* Adds support for Cypress WiFi Host Driver, LwIP TCP/IP stack and mbedTLS security for TLS

### Supported Software and Tools
This version of the library was validated for compatibility with the following Software and Tools:

| Software and Tools                                      | Version |
| :---                                                    | :----:  |
| ModusToolbox Software Environment                       | 2.0     |
| - ModusToolbox Device Configurator                      | 2.0     |
| - ModusToolbox CSD Personality in Device Configurator   | 2.0     |
| - ModusToolbox CapSense Configurator / Tuner tools      | 2.0     |
| PSoC6 Peripheral Driver Library (PDL)                   | 1.2.0   |
| GCC Compiler                                            | 7.2.1   |
| IAR Compiler                                            | 8.32    |

## Additional Information
* [WiFi middleware core README.md](./README.md)
* [WiFi middleware core API reference guide](https://cypresssemiconductorco.github.io/wifi-mw-core/api_reference_manual/html/index.html)
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
* [WiFi middleware core version](./version.txt)
