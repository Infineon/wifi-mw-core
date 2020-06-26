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

#include <string.h>
#include <stdint.h>
#include "cybsp_wifi.h"
#include "cy_network_buffer.h"
#include "whd.h"
#include "cy_lwip.h"
#include "cy_lwip_error.h"
#include "lwipopts.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/init.h"
#include "lwip/dhcp.h"
#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/ethip6.h"
#include "lwip/igmp.h"
#include "lwip/nd6.h"
#include "netif/ethernet.h"
#include "cy_result.h"
#include "whd.h"
#include "whd_wifi_api.h"
#include "whd_network_types.h"
#include "whd_buffer_api.h"
#include "cy_log.h"

#define EAPOL_PACKET_TYPE               0x888E

#define MULTICAST_IP_TO_MAC(ip)       { (uint8_t) 0x01,             \
                                        (uint8_t) 0x00,             \
                                        (uint8_t) 0x5e,             \
                                        (uint8_t) ((ip)[1] & 0x7F), \
                                        (uint8_t) (ip)[2],          \
                                        (uint8_t) (ip)[3]           \
                                      }


#ifdef ENABLE_WIFI_MIDDLEWARE_LOGS
#define wm_cy_log_msg cy_log_msg
#else
#define wm_cy_log_msg(a,b,c,...)
#endif

/******************************************************
 *               Variable Definitions
 ******************************************************/

/*
 * Currently, one network interface is supported. In future, support for multiple
 * concurrent network connections shall be added
 */
static struct netif *net_interface = NULL ;
static whd_interface_t sta_interface ;
static cy_network_activity_event_callback_t activity_callback = NULL;
static bool is_dhcp_required = false;
static bool is_netif_added  = false;
static cy_eapol_packet_handler_t internal_eapol_packet_handler = NULL;
static cy_lwip_ip_change_callback_t ip_change_callback = NULL;

struct netif *cy_lwip_get_interface(void)
{
    return net_interface ;
}


/******************************************************
 *               Static Function Declarations
 ******************************************************/
static void internal_ip_change_callback (struct netif *netif);
#if LWIP_IPV4
static void invalidate_all_arp_entries(struct netif *netif);
#endif
/******************************************************
 *               Function Definitions
 ******************************************************/
/*
 * This function takes packets from the radio driver and passes them into the
 * LwIP stack.  If the stack is not initialized, or if the LwIP stack does not
 * accept the packet, the packet is freed (dropped). If packet is of type EAPOL
 * and if EAPOL handler is registered, packet will be redirected to registered
 * handler and should be freed by EAPOL handler.
 */
void cy_network_process_ethernet_data(whd_interface_t iface, whd_buffer_t buf)
{
    uint8_t *data = whd_buffer_get_current_piece_data_pointer(iface->whd_driver, buf);
    uint16_t ethertype;

    ethertype = (uint16_t)(data[12] << 8 | data[13]);
    if (ethertype == EAPOL_PACKET_TYPE)
    {
        if( internal_eapol_packet_handler != NULL )
        {
            internal_eapol_packet_handler(buf, iface);
        }
        else
        {
            cy_buffer_release(buf, WHD_NETWORK_RX) ;
        }
    }
    else
    {
        if (net_interface != NULL)
        {
            /* Call activity handler which is registered with argument as false
             * indicating there is RX packet
             */
            if (activity_callback)
            {
                activity_callback(false);
            }
            if (net_interface->input(buf, net_interface) != ERR_OK)
                cy_buffer_release(buf, WHD_NETWORK_RX) ;
        }
        else
        {
            cy_buffer_release(buf, WHD_NETWORK_RX) ;
        }
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
 * This function takes packets from the LwIP stack and sends them down to the radio.
 * If the radio is not ready, we return and error, otherwise we add a reference to
 * the packet for the radio driver and send the packet to the radio driver.  The radio
 * driver puts the packet into a send queue and will send based on another thread.  This
 * other thread will release the packet reference once the packet is actually sent.
 */
static err_t wifioutput(struct netif *iface, struct pbuf *p)
{
    if (whd_wifi_is_ready_to_transceive((whd_interface_t)iface->state) != WHD_SUCCESS)
    {
        wm_cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_ERR, "wifi is not ready, packet not sent\n");
        return ERR_INPROGRESS ;
    }

    struct pbuf *whd_buf = pbuf_dup(p);
    if (whd_buf == NULL)
    {
        wm_cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_ERR, "failed to allocate buffer for outgoing packet\n");
        return ERR_MEM;
    }
    /* Call activity handler which is registered with argument as true
     * indicating there is TX packet
     */
    if (activity_callback)
    {
        activity_callback(true);
    }
    whd_network_send_ethernet_data((whd_interface_t)iface->state, whd_buf) ;
    return ERR_OK ;
}
#if LWIP_IPV4 && LWIP_IGMP
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
#endif
/*
 * This function is called when adding the wifi network interface to LwIP,
 * it actually performs the initialization for the netif interface.
 */
static err_t wifiinit(struct netif *iface)
{
    cy_rslt_t res ;
    whd_mac_t macaddr ;    

    /*
     * Set the MAC address of the interface
     */
    res = whd_wifi_get_mac_address(sta_interface, &macaddr) ;
    if (res != CY_RSLT_SUCCESS)
    {
        wm_cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_ERR, "whd_wifi_get_mac_address call failed, err = %lx\n", res);
        return res ;
    }  
    memcpy(&iface->hwaddr, &macaddr, sizeof(macaddr)) ;
    iface->hwaddr_len = sizeof(macaddr) ;

    /*
     * Setup the information associated with sending packets
     */
#if LWIP_IPV4
    iface->output = etharp_output ;
#endif
    iface->linkoutput = wifioutput ;
    iface->mtu = WHD_LINK_MTU ;
    iface->flags |= (NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP) ;
#ifdef LWIP_IPV6_MLD
    iface->flags |= NETIF_FLAG_MLD6;
#endif
    iface->state = sta_interface ;

    /*
     * Set the interface name for the interface
     */
    iface->name[0] = 'w' ;
    iface->name[1] = 'l' ;

#if LWIP_IPV4 && LWIP_IGMP
    netif_set_igmp_mac_filter(iface, igmp_filter) ;    
#endif
    
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
 * handle and and optional static IP address, and adds interface to LwIP.
 */
cy_rslt_t cy_lwip_add_interface(whd_interface_t iface, ip_static_addr_t *ipaddr)
{
#if LWIP_IPV4
    ip4_addr_t addr, netmask, gateway ;
#endif

    sta_interface = iface ;
    if(is_netif_added)
    {
        return CY_RSLT_LWIP_INTERFACE_EXISTS;
    }
    /* Create the network interface for the WIFI driver */
    net_interface = (struct netif *)malloc(sizeof(struct netif));
    memset(net_interface, 0, sizeof(struct netif)) ;

#if LWIP_IPV4
    /* Assign the IP address if static, otherwise, zero the IP address */
    if (ipaddr != NULL)
    {
        memcpy(&gateway, &ipaddr->gateway, sizeof(gateway));
        memcpy(&addr, &ipaddr->addr, sizeof(addr));
        memcpy(&netmask, &ipaddr->netmask, sizeof(netmask));
    }
    else
    {
        memset(&gateway, 0, sizeof(gateway));
        memset(&addr, 0, sizeof(addr));
        memset(&netmask, 0, sizeof(netmask));
    }

    /* Add the interface to LwIP and make it the default */
    if(netifapi_netif_add(net_interface, &addr, &netmask, &gateway, NULL, wifiinit, tcpip_input) != CY_RSLT_SUCCESS)
    {
        return CY_RSLT_LWIP_ERROR_ADDING_INTERFACE;
    }
#else
    if(netifapi_netif_add(net_interface, NULL, wifiinit, tcpip_input) != CY_RSLT_SUCCESS)
    {
        return CY_RSLT_LWIP_ERROR_ADDING_INTERFACE;
    }
#endif

    netifapi_netif_set_default(net_interface) ;

    /*
     * Register a handler for any address changes
     * Note : The "status" callback will also be called when the interface
     * goes up or down
     */
    netif_set_status_callback(net_interface, internal_ip_change_callback);

    is_netif_added = true;

    if(ipaddr == NULL)
    {
        is_dhcp_required = true;
    }

    return CY_RSLT_SUCCESS ;
}

cy_rslt_t cy_lwip_remove_interface(whd_interface_t iface)
{
    if(!is_netif_added)
    {
        return CY_RSLT_LWIP_INTERFACE_DOES_NOT_EXIST;
    }
    /* remove status callback */
    netif_set_remove_callback(net_interface, internal_ip_change_callback);
    /* removes the interface and sets the default interface to NULL*/
    netifapi_netif_remove(net_interface);
    free(net_interface);
    net_interface = NULL;
    is_netif_added= false;
    is_dhcp_required = false;
    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_lwip_network_up(void)
{
#if LWIP_IPV4
    ip4_addr_t ip_addr;
#endif
    /*
    * Bring up the network interface
    */
    netifapi_netif_set_up(net_interface);

    /*
    * Bring up the network link layer
    */
    netifapi_netif_set_link_up(net_interface);

#if LWIP_IPV6
    /* Wait for IPV6 address to change from tentative to valid or invalid */
    while(ip6_addr_istentative(netif_ip6_addr_state(net_interface, 0)))
    {
        /* Give LwIP time to change the state */
        cy_rtos_delay_milliseconds(ND6_TMR_INTERVAL);
    }

    /* LWIP changes state to either INVALID or VALID. Check if the state is VALID */
    if(ip6_addr_isvalid(netif_ip6_addr_state(net_interface, 0)))
    {
        wm_cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_INFO, "IPv6 Network ready IP: %s \r\n", ip6addr_ntoa(netif_ip6_addr(net_interface, 0)));
    }
    else
    {
        wm_cy_log_msg(CYLF_MIDDLEWARE, CY_LOG_INFO, "IPv6 network not ready \r\n");
    }
#endif

#if LWIP_IPV4
    if(is_dhcp_required)
    {
        /* TO DO :  Save the current power save state */

        /* TO DO :  Disable power save for the DHCP exchange */

        /*
         * For DHCP only, we should reset netif IP address
         * We don't want to re-use previous netif IP address
         * given from previous DHCP session
         */
        ip4_addr_set_zero(&ip_addr);
        netif_set_ipaddr(net_interface, &ip_addr);

        /* TO DO : DHCPV6 need to be handled when we support IPV6 addresses other than the link local address */
        /* Start DHCP */
        if(netifapi_dhcp_start(net_interface) != CY_RSLT_SUCCESS)
        {
            return CY_RSLT_LWIP_ERROR_STARTING_DHCP;
        }

        /* Wait a little to allow DHCP a chance to complete */
        vTaskDelay(10);
        /*TO DO : enable power save */
    }
#endif

    return CY_RSLT_SUCCESS;
}

cy_rslt_t cy_lwip_network_down(void)
{
#if LWIP_IPV4
    if(is_dhcp_required)
    {
        netifapi_dhcp_release_and_stop(net_interface);
        vTaskDelay(400);
    }

    dhcp_cleanup(net_interface);
#endif
    /*
    * Bring down the network link layer
    */
    netifapi_netif_set_link_down(net_interface);

    /*
    * Bring down the network interface
    */
    netifapi_netif_set_down(net_interface);

    /* TO DO : clear all ARP cache */

    /** TO DO:
     *  Kick the radio chip if it's in power save mode in case the link down event is due to missing beacons.
     *  Setting the chip to the same power save mode is sufficient.
     */
    return CY_RSLT_SUCCESS;
}

void cy_lwip_register_ip_change_cb(cy_lwip_ip_change_callback_t cb)
{
    ip_change_callback = cb;
}


/*
 * This functions helps to register/unregister callback for network activity
 */
void cy_network_activity_register_cb(cy_network_activity_event_callback_t cb)
{
    /* update the activity callback with the argument passed */
    activity_callback = cb;
}
#if LWIP_IPV4
cy_rslt_t cy_lwip_dhcp_renew(void)
{
    /* Invalidate ARP entries */
    netifapi_netif_common(net_interface, (netifapi_void_fn) invalidate_all_arp_entries, NULL );

    /* DHCP renewal*/
    netifapi_netif_common(net_interface, (netifapi_void_fn)dhcp_renew, NULL);

    vTaskDelay(100);
    return CY_RSLT_SUCCESS;
}
/**
 * Remove all ARP table entries of the specified netif.
 * @param netif points to a network interface
 */
static void invalidate_all_arp_entries(struct netif *netif)
{
     /*free all the entries in arp list */
    etharp_cleanup_netif(netif);

}
#endif

/* Used to register callback for EAPOL packets */
whd_result_t cy_eapol_register_receive_handler( cy_eapol_packet_handler_t eapol_packet_handler )
{
    internal_eapol_packet_handler = eapol_packet_handler;
    return WHD_SUCCESS;
}

static void internal_ip_change_callback (struct netif *netif)
{
    /* notify wcm about ip change */
    if(ip_change_callback != NULL)
    {
        ip_change_callback(NULL);
    }
}
