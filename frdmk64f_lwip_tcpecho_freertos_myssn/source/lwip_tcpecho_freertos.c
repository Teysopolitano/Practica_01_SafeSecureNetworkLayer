/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/opt.h"

#if LWIP_NETCONN

#include "tcpecho.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"
#include "board.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "clock_config.h"

/*Safe and secure network*/
#include "ssnet.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* IP address configuration. */
#define configIP_ADDR0 192
#define configIP_ADDR1 168
#define configIP_ADDR2 0
#define configIP_ADDR3 102

/* Netmask configuration. */
#define configNET_MASK0 255
#define configNET_MASK1 255
#define configNET_MASK2 255
#define configNET_MASK3 0

/* Gateway address configuration. */
#define configGW_ADDR0 192
#define configGW_ADDR1 168
#define configGW_ADDR2 0
#define configGW_ADDR3 100

/* MAC address configuration. */
#define configMAC_ADDR                     \
    {                                      \
        0x02, 0x12, 0x13, 0x10, 0x15, 0x11 \
    }

/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS

/* System clock name. */
#define EXAMPLE_CLOCK_NAME kCLOCK_CoreSysClk


#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void tcpecho_server_ssnet_thread(void *arg);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    static struct netif netif;
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
    static mem_range_t non_dma_memory[] = NON_DMA_MEMORY_ARRAY;
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    ip4_addr_t netif_ipaddr, netif_netmask, netif_gw;
    ethernetif_config_t enet_config = {
        .phyAddress = EXAMPLE_PHY_ADDRESS,
        .clockName  = EXAMPLE_CLOCK_NAME,
        .macAddress = configMAC_ADDR,
#if defined(FSL_FEATURE_SOC_LPC_ENET_COUNT) && (FSL_FEATURE_SOC_LPC_ENET_COUNT > 0)
        .non_dma_memory = non_dma_memory,
#endif /* FSL_FEATURE_SOC_LPC_ENET_COUNT */
    };

    SYSMPU_Type *base = SYSMPU;
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Disable SYSMPU. */
    base->CESR &= ~SYSMPU_CESR_VLD_MASK;

    IP4_ADDR(&netif_ipaddr, configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3);
    IP4_ADDR(&netif_netmask, configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3);
    IP4_ADDR(&netif_gw, configGW_ADDR0, configGW_ADDR1, configGW_ADDR2, configGW_ADDR3);

    tcpip_init(NULL, NULL);

    netifapi_netif_add(&netif, &netif_ipaddr, &netif_netmask, &netif_gw, &enet_config, EXAMPLE_NETIF_INIT_FN,
                       tcpip_input);
    netifapi_netif_set_default(&netif);
    netifapi_netif_set_up(&netif);

    PRINTF("\r\n************************************************\r\n");
    PRINTF(" TCP Echo example\r\n");
    PRINTF("************************************************\r\n");
    PRINTF(" IPv4 Address     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_ipaddr)[0], ((u8_t *)&netif_ipaddr)[1],
           ((u8_t *)&netif_ipaddr)[2], ((u8_t *)&netif_ipaddr)[3]);
    PRINTF(" IPv4 Subnet mask : %u.%u.%u.%u\r\n", ((u8_t *)&netif_netmask)[0], ((u8_t *)&netif_netmask)[1],
           ((u8_t *)&netif_netmask)[2], ((u8_t *)&netif_netmask)[3]);
    PRINTF(" IPv4 Gateway     : %u.%u.%u.%u\r\n", ((u8_t *)&netif_gw)[0], ((u8_t *)&netif_gw)[1],
           ((u8_t *)&netif_gw)[2], ((u8_t *)&netif_gw)[3]);
    PRINTF("************************************************\r\n");

    sys_thread_new("tcpecho_task", tcpecho_server_ssnet_thread, NULL, 1024, 4);

    vTaskStartScheduler();

    /* Will not get here unless a task calls vTaskEndScheduler ()*/
    return 0;
}
#endif

void tcpecho_server_ssnet_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* CRC data */
  uint32_t checksum32;

  /* Create a new connection identifier. */
  /* Bind connection to well known port number 7. */
#if LWIP_IPV6
  conn = netconn_new(NETCONN_TCP_IPV6);
  netconn_bind(conn, IP6_ADDR_ANY, 7);
#else /* LWIP_IPV6 */
   conn = netconn_new(NETCONN_TCP);
  netconn_bind(conn, IP_ADDR_ANY, 7);
#endif /* LWIP_IPV6 */
  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);

  /* Tell connection to go into listening mode. */
  netconn_listen(conn);

  while (1) {

    /* Grab new connection. */
    err = netconn_accept(conn, &newconn);
//    PRINTF("Accepted new connection %p\r\n", newconn);

    /* Process the new connection. */
    if (err == ERR_OK) {
      struct netbuf *buf;
      void *data;
      uint8_t *data_msg;
      u16_t len;
	  uint8_t chksum[4];
      uint8_t *p_chksum = chksum;
      uint32_t chksum_bytes;
      uint32_t *p_chksum_bytes = &chksum_bytes;

      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
//        PRINTF("Received\r\n");

        do {

        	netbuf_data(buf, &data, &len);

        	/*Get the message body*/
        	data_msg =(uint8_t *) data;

        	uint8_t message[len-4];
        	uint8_t *p_message = message;

        	PRINTF("Received data: \r\n");
        	for(int i=0; i<len; i++) {
        	    PRINTF("0x%02x,", data_msg[i]);
        			}
        	PRINTF("\r\n");

        	/*Get body and CRC part*/
        	uint8_t k = 3;
        	for (int i=0; i<len; i++)
        	{
        		if (i<(len-4))
        		p_message[i] = data_msg[i];
        		else
        		{
        		p_chksum[k] = data_msg[i];
        		k--;
        		}
        	}

        	chksum_bytes = chksum[3]| (chksum[2]<<8 | chksum[1] <<16 | chksum[0] <<24);

        	PRINTF("Message body: \r\n");
          	for(int i=0; i<len-4; i++) {
               	PRINTF("0x%02x,", p_message[i]);
                }
            PRINTF("\r\n");

            PRINTF("CHKSUM: ");
            for(int i=0; i<4; i++) {
            PRINTF("0x%02x,", p_chksum[i]);
            }
            PRINTF("\r\n");
            PRINTF("Checksum bytes: 0x%08x\r\n", chksum_bytes);

        	/*Calculate CRC32 for body message*/
            checksum32 = calculate_crc32(p_message, len, config, base);
        	PRINTF("CRC-32: 0x%08x\r\n", checksum32);

        	/*Compare calculated checksum with the one contained in the message*/
        	if (chksum_bytes != checksum32)
        	{
        		PRINTF("\n Checksum error! \n");
        	}
        	else
        	{
           	    /*Decrypt message and send it back to client*/
        	    uint8_t *p_dec_msg = malloc(len-4);
        		p_dec_msg = decrypt(p_message);

        		/*Encrypt message again*/
        	    uint8_t *p_enc_msg = malloc(len);
        	    p_enc_msg = encrypt(p_dec_msg);

        	    /*Calculate CRC32 of encrypted message*/
        		checksum32 = calculate_crc32(p_enc_msg, len, config, base);
        		memcpy(chksum, &checksum32, sizeof(uint32_t));
        		chksum_bytes = chksum[3]| (chksum[2]<<8 | chksum[1] <<16 | chksum[0] <<24);

        	    /*Concatenate encrypted message and checksum*/
        		data_msg = p_enc_msg +  *p_chksum_bytes;

        	    /*Send data*/
        	    err = netconn_write(newconn, data, len, NETCONN_COPY);
        	}

#if 0
            if (err != ERR_OK) {
              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
            }
#endif
        } while (netbuf_next(buf) >= 0);
        netbuf_delete(buf);
      }
      /*printf("Got EOF, looping\n");*/
      /* Close connection and discard connection identifier. */
      netconn_close(newconn);
      netconn_delete(newconn);
    }
  }
}
/*-----------------------------------------------------------------------------------*/
