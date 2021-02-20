/*
 * ssnet.h
 *
 *  Created on: 13 feb. 2021
 *      Author: Luis Teyssier
 */

#ifndef SSNET_H_
#define SSNET_H_

#include "aes.h"
#include "fsl_crc.h"
#include "lwip/sys.h"
#include "ssnet_conf.h"


uint8_t * encrypt(uint8_t *message);
uint8_t * decrypt(uint8_t *message);
void InitCrc32(CRC_Type *base, crc_config_t config);
uint32_t calculate_crc32(uint8_t *message, size_t length, crc_config_t config, CRC_Type *base);

#endif /* SSNET_H_ */
