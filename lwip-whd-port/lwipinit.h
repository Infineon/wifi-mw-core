/*
 * Copyright 2019 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
* \addtogroup group_lwip_whd_port lwIP and WHD port
* \{
* Functions for dealing with linking the lwIP TCP/IP stack with WiFi Host Driver
*
* \defgroup group_lwip_whd_port_functions Functions
* \defgroup group_lwip_whd_port_structures Structures
*/
#pragma once

#include <whd_wifi_api.h>
#include <cy_result.h>
#include <lwip/ip_addr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
* \addtogroup group_lwip_whd_port_structures
* \{
*/

/**
 * This structure represents a static IP address
 */
typedef struct ip_static
{    
    ip_addr_t addr;    /**< The IP address for the network interface */
    ip_addr_t netmask; /**< The netmask for the network interface */
    ip_addr_t gateway; /**< The default gateway for network traffic */

} ip_static_addr_t;

/** \} group_lwip_whd_port_structures */

/**
* \addtogroup group_lwip_whd_port_functions
* \{
*/

/**
 * Add a WHD wifi interface to LWiP and bring it up
 * This is the entry point in this file.  This function takes a WHD radio driver
 * handle and an optional static IP address, and brings up the LWiP network interface.
 *
 * @param[in] iface Interface to be added to lwIP
 * @param[in] ipaddr IPv4/IPv6 address information associated with the interface
 * @return CY_RESULT_SUCCESS for successful addition to lwIP or error otherwise
 */
extern cy_rslt_t add_interface_to_lwip(whd_interface_t iface, ip_static_addr_t *ipaddr) ;

/**
 * Return the single LWiP network interface.
 *
 * @return netif structure of the WHD interface
 */
extern struct netif *get_lwip_interface() ;

/**
 *
 * This function takes packets from the radio driver and passes them into the
 * lwIP stack.  If the stack is not initialized, or if the lwIP stack does not
 * accept the packet, the packet is freed (dropped).
 * This function will be registered as part of the whd_netif_funcs defined in whd.h
 * of the WiFi Host Driver
 *
 * @param[in] iface WiFi interface
 * @param[in] buf Packet received from the radio driver
 * 
 */
extern void cy_network_process_ethernet_data(whd_interface_t iface, whd_buffer_t buf);

/** \} group_lwip_whd_port_functions */
#ifdef __cplusplus
}
#endif
/** \} group_lwip_whd_port */

