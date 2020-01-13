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
#include "mbedtls/config.h"
#include "mbedtls_net_sockets.h"
#include <FreeRTOS.h>
#include <task.h>
#include <cy_syslib.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int strToPort(const char *port)
{
    int portno = atoi(port) ;
    while (*port)
    {
        if (!isdigit(*port))
            return -1 ;

        port++ ;
    }

    return portno ;
}

void mbedtls_net_init( mbedtls_net_context *ctx )
{
    ctx->connection = NULL ;
    ctx->blocking = false ;
    ctx->rddata = NULL ;
    ctx->used = 0 ;
}

/*
 * Initiate a TCP connection with host:port and the given protocol
 */
int mbedtls_net_connect( mbedtls_net_context *ctx, const char *host, const char *port, int proto )
{
    ip_addr_t addr ;
    err_t ret ;
    int portno ;
    enum netconn_type type ;

    portno = strToPort(port) ;
    if (portno == -1)
        return MBEDTLS_ERR_NET_BAD_INPUT_DATA ;

    if (proto == MBEDTLS_NET_PROTO_TCP)
        type = NETCONN_TCP ;
    else
        type = NETCONN_UDP ;

    if (netconn_gethostbyname_addrtype(host, &addr, NETCONN_DNS_IPV4_IPV6) != ERR_OK)
    {
        return MBEDTLS_ERR_NET_UNKNOWN_HOST ;
    }
    
    ctx->connection = netconn_new(type) ;
    if (ctx->connection == NULL)
        return MBEDTLS_ERR_NET_CONNECT_FAILED ;

    ret = netconn_connect(ctx->connection, &addr, portno) ;
    if (ret != ERR_OK)
    {
        netconn_delete(ctx->connection) ;
        ctx->connection = NULL ;
        return MBEDTLS_ERR_NET_CONNECT_FAILED ;
    }
    
    return 0 ;
}

/*
 * Create a listening socket on bind_ip:port
 */
int mbedtls_net_bind( mbedtls_net_context *ctx, const char *bind_ip, const char *port, int proto )
{
    err_t ret ;
    int portno ;
    enum netconn_type type ;
    ip_addr_t addr ;

    portno = strToPort(port) ;
    if (portno == -1)
        return MBEDTLS_ERR_NET_BAD_INPUT_DATA ;

    if (proto == MBEDTLS_NET_PROTO_TCP)
        type = NETCONN_TCP ;
    else
        type = NETCONN_UDP ;        

    if (bind_ip == NULL)
        addr = ip_addr_any ;
    else
    {    
        if (netconn_gethostbyname_addrtype(bind_ip, &addr, NETCONN_DNS_IPV4_IPV6) != ERR_OK)
        {
            return MBEDTLS_ERR_NET_UNKNOWN_HOST ;
        }
    }       

    ctx->connection = netconn_new(type) ;
    if (ctx->connection == NULL)
        return MBEDTLS_ERR_NET_CONNECT_FAILED ;     

    ret = netconn_bind(ctx->connection, &addr, portno) ;
    if (ret != ERR_OK)
    {
        netconn_delete(ctx->connection) ;
        ctx->connection = NULL ;
        return MBEDTLS_ERR_NET_BIND_FAILED ;
    }

    ret = netconn_listen(ctx->connection) ;
    if (ret != ERR_OK)
    {
        netconn_delete(ctx->connection) ;
        ctx->connection = NULL ;
        return MBEDTLS_ERR_NET_BIND_FAILED ;        
    }

    return 0 ;
}

/*
 * Accept a connection from a remote client
 */
int mbedtls_net_accept( mbedtls_net_context *bind_ctx, mbedtls_net_context *client_ctx, void *client_ip, size_t buf_size, size_t *ip_len )
{
    /* To-Do: To retrieve and provide remote client ip address
     * to the caller of this API, LwIP socket APIs  need to be used */
    UNUSED_PARAMETER(buf_size);
    UNUSED_PARAMETER(client_ip);
    err_t ret ;

    ret = netconn_accept(bind_ctx->connection, &client_ctx->connection) ;
    if (ret != ERR_OK)
        return MBEDTLS_ERR_NET_ACCEPT_FAILED ;

    /* Caller checks the ip_len before using client_ip */
    if (ip_len != NULL)
    {
        *ip_len = 0 ;
    }

    return 0 ;
}

/*
 * Set the socket blocking or non-blocking
 */
int mbedtls_net_set_block( mbedtls_net_context *ctx )
{
    /* To-Do: Invoke lwIP netconn API */
    ctx->blocking = true ;
    return 0 ;
}

int mbedtls_net_set_nonblock( mbedtls_net_context *ctx )
{
    /* To-Do: Invoke lwIP netconn API */
    ctx->blocking = false ;
    return 0 ;
}

/*
 * Check if data is available on the socket
 */
int mbedtls_net_poll( mbedtls_net_context *ctx, uint32_t rw, uint32_t timeout )
{
    /* To-Do: Implement the logic to return appropriate read/write flags */
    return MBEDTLS_ERR_NET_POLL_FAILED;
}

/*
 * Portable usleep helper
 * 
 * We use this instead of an RTOS sleep because its micro-seconds.
 */
void mbedtls_net_usleep( unsigned long usec )
{
    Cy_SysLib_DelayUs(usec) ;
}

/*
 * Read at most 'len' characters
 */
int mbedtls_net_recv( void *ctx, unsigned char *buf, size_t len )
{
    err_t ret ;
    struct pbuf *p ;
    size_t outoffset = 0 ;
    size_t totalread = 0 ;
    size_t toread = 0 ;

    mbedtls_net_context *myctx = (mbedtls_net_context *)ctx ;

    do {
        if (myctx->rddata == NULL)
        {
            ret = netconn_recv_tcp_pbuf(myctx->connection, &p) ;
            if (ret != ERR_OK)
            {
                printf("netconn_recv_tcp_pbuf failed with error %d\n", ret);
                return  MBEDTLS_ERR_NET_RECV_FAILED ;
            }
            if(p->len == 0)
            {
                myctx->rddata = NULL;
                continue;
            }

            myctx->rddata = p ;
            myctx->used = 0 ;            
        }

        /*
        * This is the data left to read
        */
        toread = len - totalread ;
        if (toread > myctx->rddata->len - myctx->used)
            toread = myctx->rddata->len - myctx->used ;

        /*
        * Copy the data out
        */
        memcpy(buf + outoffset, (uint8_t *)myctx->rddata->payload + myctx->used, toread) ;

        /*
        * Mark the data used from the current p buf
        */
        myctx->used += toread ;

        /*
        * Move the output pointer for the output buffer
        */
        outoffset += toread ;

        /*
        * Keep track of the total read
        */
        totalread += toread ;

        /*
        * If we used up the current buffer, mark the context
        * as not having read data.  This will force another network
        * read the next time through the loop
        */
        if (myctx->used == myctx->rddata->len)
        {
            pbuf_free(myctx->rddata) ;
            myctx->rddata = NULL ;
        }

    } while (totalread < len) ;

    return len ;
}

/*
 * Read at most 'len' characters, blocking for at most 'timeout' ms
 */
int mbedtls_net_recv_timeout( void *ctx, unsigned char *buf, size_t len, uint32_t timeout )
{
    /*
     * TODO: not used in current configuration
     */
    while (1) ;

    return 0 ;
}

/*
 * Write at most 'len' characters
 */
int mbedtls_net_send( void *ctx, const unsigned char *buf, size_t len )
{
    err_t ret ;

    mbedtls_net_context *myctx = (mbedtls_net_context *)ctx ;
    ret = netconn_write(myctx->connection, buf, len, 0) ;
    if (ret != ERR_OK)
    {
        if (ret == ERR_RST)
            return MBEDTLS_ERR_NET_CONN_RESET ;

        return  MBEDTLS_ERR_NET_SEND_FAILED ;
    }
    
    return len ;
}

/*
 * Gracefully close the connection
 */
void mbedtls_net_free( mbedtls_net_context *ctx )
{
    if (ctx->rddata != NULL)
        pbuf_free(ctx->rddata) ;
    
    if (ctx->connection != NULL)
    {
        netconn_close(ctx->connection) ;
        netconn_delete(ctx->connection) ;
    }
}
