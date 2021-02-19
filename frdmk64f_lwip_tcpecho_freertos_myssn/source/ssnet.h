/*
 * ssnet.h
 *
 *  Created on: 13 feb. 2021
 *      Author: Luis Teyssier
 */

#ifndef SSNET_H_
#define SSNET_H_

#include "lwip/opt.h"
#include "tcpecho.h"
#include "lwip/netifapi.h"
#include "lwip/tcpip.h"
#include "netif/ethernet.h"
#include "enet_ethernetif.h"
#include "aes.h"
#include "fsl_crc.h"

#include "lwip/opt.h"


#include "lwip/sys.h"
#include "lwip/api.h"


uint8_t * encrypt(uint8_t *message);
uint8_t * decrypt(uint8_t *message);
void InitCrc32(CRC_Type *base, uint32_t seed);
uint32_t calculate_crc32(uint8_t *message, size_t length);

void tcpecho_server_ssnet_thread(void *arg);


#endif /* SSNET_H_ */
