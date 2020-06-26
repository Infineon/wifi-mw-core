/*
 * Copyright 2020 Cypress Semiconductor Corporation
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
 * EAPOL handler to receive EAPOL data. buffer should be freed by EAPOL handler
 *
 * @param[in] buffer     buffer received from WHD
 * @param[in] interface  WHD interface
 *
 */
typedef void (*cy_eapol_packet_handler_t) (whd_buffer_t buffer, whd_interface_t interface);

/**
 * Add a WHD wifi interface to lwIP and bring it up
 * This is the entry point in this file.  This function takes a WHD radio driver
 * and adds the interface to lwIP, configures the optional static ip and
 * registers to IP change callback. Currently only one interface is added
 * and this is added as default interface. \n
 * Note : In future additionl interfaces will be supported.
 *
 * @param[in] iface Interface to be added to lwIP
 * @param[in] ipaddr IPv4/IPv6 address information associated with the interface. IP address need to be passed in network byte order.
 * @return CY_RESULT_SUCCESS for successful addition to lwIP or error otherwise
 */
cy_rslt_t cy_lwip_add_interface(whd_interface_t iface, ip_static_addr_t *ipaddr) ;

/**
 * Removes a WHD wifi interface from lwIP.
 * This function takes a WHD radio driver handle and removes the lwIP network interface.
 *
 * @param[in] iface Interface to be removed from lwIP
 *
 * @return CY_RSLT_SUCCESS if successful, failure code otherwise.
 */
cy_rslt_t cy_lwip_remove_interface(whd_interface_t iface);

/**
 * Return the single lwIP network interface.
 *
 * @return netif structure of the WHD interface
 */
struct netif *cy_lwip_get_interface(void) ;

/**
 * This function brings up the network link layer and setups the network interface
 * and starts DHCP if required. Also waits for IPV6 link local address to be configured, if IPv6 is enabled.
 * Once the IPv6 link local address is ready, it prints the address else prints it's status.
 *
 *  @return CY_RSLT_SUCCESS if successful, failure code otherwise.
 */
cy_rslt_t cy_lwip_network_up(void);

/**
 * This function brings down the network interface, brings down the network link layer
 * and stops DHCP
 *
 * @return CY_RSLT_SUCCESS if successful, failure code otherwise.
 */
cy_rslt_t cy_lwip_network_down(void);

#if LWIP_IPV4
/**
 * This function Invalidates all the ARP entries and renews the DHCP, typically
 * used when handshake failure occurs
 *
 * @return CY_RSLT_SUCCESS if successful, failure code otherwise.
 */
cy_rslt_t cy_lwip_dhcp_renew(void);
#endif
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

/**
 * Network activity callback function prototype
 * Callback function which can be registered/unregistered for any network activity
 * needs to be of this prototype
 */
typedef void (*cy_network_activity_event_callback_t)(bool callback_arg);


/**
 * IP change callback function prototype
 * Callback function which can be registered to receive IP changes
 * needs to be of this prototype
 */
typedef void (*cy_lwip_ip_change_callback_t)(void *data);

/**
 * This function helps to register/unregister callback fn for any TX/RX packets.
 * Passing "NULL" as cb will deregister the activity callback
 *
 * @param[in] cb Network activity callback function
 * 
 */
void cy_network_activity_register_cb(cy_network_activity_event_callback_t cb);

/**
 * This function helps to register/unregister for any IP changes from lwIP.
 * Passing "NULL" as callback will deregister the IP changes callback
 *
 * @param[in] cb IP change callback function
 *
 */
void cy_lwip_register_ip_change_cb(cy_lwip_ip_change_callback_t cb);

/**
 *
 * This API allows registering callback functions to receive EAPOL packets
 * from WHD. If callback is registered and received packet is EAPOL packet
 * then it will be directly redirected to registered callback. passing "NULL"
 * as handler will de-register the previously registered callback
 *
 * @param[in] eapol_packet_handler
 *
 * @return whd_result_t
 *
 */
whd_result_t cy_eapol_register_receive_handler( cy_eapol_packet_handler_t eapol_packet_handler );


/** \} group_lwip_whd_port_functions */
#ifdef __cplusplus
}
#endif
/** \} group_lwip_whd_port */

