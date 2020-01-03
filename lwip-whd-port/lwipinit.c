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

#include "cybsp_wifi.h"
#include "cy_network_buffer.h"
#include "whd.h"
#include "lwipinit.h"
#include "lwipopts.h"
#include <lwip/netif.h>
#include <lwip/netifapi.h>
#include <lwip/init.h>
#include <lwip/dhcp.h>
#include <lwip/etharp.h>
#include <lwip/tcpip.h>
#include <lwip/ethip6.h>
#include <lwip/igmp.h>
#include <netif/ethernet.h>
#include <cy_result.h>
#include <whd.h>
#include <whd_wifi_api.h>
#include <whd_network_types.h>
#include <string.h>
#include <stdint.h>

#define MULTICAST_IP_TO_MAC(ip)       { (uint8_t) 0x01,             \
                                        (uint8_t) 0x00,             \
                                        (uint8_t) 0x5e,             \
                                        (uint8_t) ((ip)[1] & 0x7F), \
                                        (uint8_t) (ip)[2],          \
                                        (uint8_t) (ip)[3]           \
                                      }


/*
 * Currently, one network interface is supported. In future, support for multiple
 * concurrent network connections shall be added
 */
static struct netif *default_interface = NULL ;
static whd_interface_t wifi_interface ;

struct netif *get_lwip_interface()
{
    return default_interface ;
}

/*
 * This function takes packets from the radio driver and passes them into the
 * LWiP stack.  If the stack is not initialized, or if the LWiP stack does not
 * accept the packet, the packet is freed (dropped).
 */
void cy_network_process_ethernet_data(whd_interface_t iface, whd_buffer_t buf)
{
    if (default_interface != NULL)
    {
        if (default_interface->input(buf, default_interface) != ERR_OK)
            cy_buffer_release(buf, WHD_NETWORK_RX) ;
    }
    else
    {
        cy_buffer_release(buf, WHD_NETWORK_RX) ;
    }
}

/* This function creates duplicate pbuf of input pbuf */
static struct pbuf *pbuf_dup(const struct pbuf *orig)
{
    struct pbuf *p = pbuf_alloc(PBUF_LINK, orig->tot_len, PBUF_RAM);
    if (p != NULL)
    {
        pbuf_copy(p, orig);
        p->flags = orig->flags;
    }
    return p;
}

/*
 * This function takes packets from the LWiP stack and sends them down to the radio.
 * If the radio is not ready, we return and error, otherwise we add a reference to
 * the packet for the radio driver and send the packet to the radio driver.  The radio
 * driver puts the packet into a send queue and will send based on another thread.  This
 * other thread will release the packet reference once the packet is actually sent.
 */
static err_t wifioutput(struct netif *iface, struct pbuf *p)
{
    if (whd_wifi_is_ready_to_transceive((whd_interface_t)iface->state) != WHD_SUCCESS)
    {
        printf("wifi is not ready, packet not sent\n") ;
        return ERR_INPROGRESS ;
    }

    struct pbuf *whd_buf = pbuf_dup(p);
    if (whd_buf == NULL)
    {
        printf("failed to allocate buffer for outgoing packet\n");
        return ERR_MEM;
    }
    whd_network_send_ethernet_data((whd_interface_t)iface->state, whd_buf) ;
    return ERR_OK ;
}

/*
 * This function is used to respond to IGMP (group management) requests.
 */
static err_t igmp_filter(struct netif *iface, const ip4_addr_t *group, enum netif_mac_filter_action action)
{
    whd_mac_t mac = { MULTICAST_IP_TO_MAC((uint8_t*)group) };

    switch ( action )
    {
        case NETIF_ADD_MAC_FILTER:
            if ( whd_wifi_register_multicast_address( (whd_interface_t)iface->state, &mac ) != CY_RSLT_SUCCESS )
            {
                return ERR_VAL;
            }
            break;

        case NETIF_DEL_MAC_FILTER:
            if ( whd_wifi_unregister_multicast_address( (whd_interface_t)iface->state, &mac ) != CY_RSLT_SUCCESS )
            {
                return ERR_VAL;
            }
            break;

        default:
            return ERR_VAL;
    }

    return ERR_OK;

}

/*
 * This function is called when adding the wifi network interface to LWiP.  it actually performs
 * the initializtaion for the netif interface.
 */
static err_t wifiinit(struct netif *iface)
{
    cy_rslt_t res ;
    whd_mac_t macaddr ;    

    /*
     * Set the MAC address of the interface
     */
    res = whd_wifi_get_mac_address(wifi_interface, &macaddr) ;
    if (res != CY_RSLT_SUCCESS)
    {
        printf("initLWIP: whd_wifi_get_mac_address call failed, err = %lx", res) ;
        return res ;
    }  
    memcpy(&iface->hwaddr, &macaddr, sizeof(macaddr)) ;
    iface->hwaddr_len = sizeof(macaddr) ;

    /*
     * Setup the information associated with sending packets
     */
    iface->output = etharp_output ;
    iface->linkoutput = wifioutput ;
    iface->mtu = WHD_LINK_MTU ;
    iface->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP) ;
    iface->state = wifi_interface ;

    /*
     * Set the interface name for the interface
     */
    iface->name[0] = 'w' ;
    iface->name[1] = 'l' ;

    netif_set_igmp_mac_filter(iface, igmp_filter) ;    
    
#if LWIP_IPV6 == 1
    /*
     * Filter output packets for IPV6 through the ethernet output
     * function for IPV6.
     */
    iface->output_ip6 = ethip6_output ;

    /*
     * Automatically generate a unicast IP address based on
     * neighbor discovery.
     */
    iface->ip6_autoconfig_enabled = 1 ;

    /*
     * Create a link local IPV6 address
     */
    netif_create_ip6_linklocal_address(iface, 1);

    /*
     * Tell the radio that we want to listen to solicited-node multicast
     * packets.  These packets are part of the IPV6 neighbor discovery
     * process.
     */
    macaddr.octet[0] = 0x33 ;
    macaddr.octet[1] = 0x33 ;
    macaddr.octet[2] = 0xff ;  
    whd_wifi_register_multicast_address(iface->state, &macaddr) ;

    /*
     * Tell the radio that we want to listen to the multicast address
     * that targets all IPV6 devices.  These packets are part of the IPV6
     * neighbor discovery process.
     */
    memset(&macaddr, 0, sizeof(macaddr)) ;
    macaddr.octet[0] = 0x33 ;
    macaddr.octet[1] = 0x33 ;
    macaddr.octet[5] = 0x01 ;
    whd_wifi_register_multicast_address(iface->state, &macaddr) ;
#endif

    return 0 ;
}

/*
 * This is the main entry point in this file.  This function takes a WHD radio driver
 * handle and and optional static IP address, and brings up the LWiP network interface.
 */
cy_rslt_t add_interface_to_lwip(whd_interface_t iface, ip_static_addr_t *ipaddr)
{
    struct netif *niface ;
    ip4_addr_t addr, netmask, gateway ;

    wifi_interface = iface ;

    /*
    * Create the network interface for the WIFI driver
    */
    niface = (struct netif *)malloc(sizeof(struct netif)) ;
    memset(niface, 0, sizeof(struct netif)) ;

    /*
    * Assign the IP address if static, otherwise, zero the IP address
    */
    if (ipaddr != NULL)
    {
        memcpy(&gateway, &ipaddr->gateway, sizeof(gateway)) ;
        memcpy(&addr, &ipaddr->addr, sizeof(addr)) ;
        memcpy(&netmask, &ipaddr->netmask, sizeof(netmask)) ;
    }
    else
    {
        memset(&gateway, 0, sizeof(gateway)) ;
        memset(&addr, 0, sizeof(addr)) ;
        memset(&netmask, 0, sizeof(netmask)) ;
    }

    /* Add the ineterface to LWiP and make it the default */
    netifapi_netif_add(niface, &addr, &netmask, &gateway, NULL, wifiinit, ethernet_input) ;
    netifapi_netif_set_default(niface) ;
    default_interface = niface ;

    /*
    * Bring up the network link layer
    */
    netifapi_netif_set_link_up(niface) ;

    /*
    * Bring up the network interface
    */
    netifapi_netif_set_up(niface) ;

    if (ipaddr == NULL)
    {
        /*
        * Start the DHCP client to get an IP address if a static
        * IP adress was not provided.
        */
        netifapi_dhcp_start(niface) ;
    }
    
    return CY_RSLT_SUCCESS ;
}
