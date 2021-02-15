/*
 * ssnet.h
 *
 *  Created on: 13 feb. 2021
 *      Author: luist
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



uint8_t encrypt(uint8_t *message);
uint8_t decrypt(uint8_t *message);


#endif /* SSNET_H_ */
