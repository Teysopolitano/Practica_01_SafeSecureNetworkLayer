/*
 * ssnet_conf.h
 *
 *  Created on: 19 feb. 2021
 *      Author: Luis Teyssier
 */

#ifndef SSNET_CONF_H_
#define SSNET_CONF_H_

#include "fsl_crc.h"
#include "aes.h"

/*CRC32 variables*/
extern crc_config_t config;
extern CRC_Type *base;

///*AES variables*/
extern uint8_t key[];
extern uint8_t iv[];
extern struct AES_ctx ctx;


#endif /* SSNET_CONF_H_ */
