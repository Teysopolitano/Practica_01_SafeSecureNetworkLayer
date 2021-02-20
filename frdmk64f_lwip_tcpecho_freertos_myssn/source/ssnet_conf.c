/*
 * ssnet_conf.c
 *  AES128 and CRC32 Configuration file
 *  Created on: 19 feb. 2021
 *      Author: Luis Teyssier
 */

#include  "ssnet_conf.h"

/*CRC32 Configuration*/
crc_config_t config=
{
	0x04C11DB7U,        //polynomial
	0xFFFFFFFFU,        //seed
	true,               //reflectIn
	true,               //reflectOut
	true,               //complementChecksum
	kCrcBits32,         //crcBits
	kCrcFinalChecksum   //crcResult
};

CRC_Type *base = CRC0;


/*AES Key and IV definition*/
uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
struct AES_ctx ctx;
