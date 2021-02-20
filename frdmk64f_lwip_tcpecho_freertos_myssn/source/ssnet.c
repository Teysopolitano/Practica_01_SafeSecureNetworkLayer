/*
 * ssnet.c
 *
 * Safe and Secure Network layer implementation
 *
 *  Created on: 13 feb. 2021
 *      Author: Luis Teyssier
 */

#include "ssnet.h"

//uint32_t checksum32;


/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
void InitCrc32(CRC_Type *base, crc_config_t config)
{
    CRC_Init(base, &config);
}

/*Encrypt message using AES128 algorithm*/
uint8_t * encrypt(uint8_t *message)
{
	size_t message_len, padded_len;
	static uint8_t padded_msg[512] = {0};

	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

	/* To encrypt an array its lenght must be a multiple of 16 so we add zeros */
	message_len = strlen(message);
	padded_len = message_len + (16 - (message_len%16) );
	memcpy(padded_msg, message, message_len);

	PRINTF("Message: ");
			for(int i=0; i<padded_len; i++) {
				PRINTF("0x%02x,", padded_msg[i]);
			}
	PRINTF("\r\n");

	AES_CBC_encrypt_buffer(&ctx, padded_msg, padded_len);

	PRINTF("Encrypted Message: ");
		for(int i=0; i<padded_len; i++) {
			PRINTF("0x%02x,", padded_msg[i]);
		}
	PRINTF("\r\n");

	return padded_msg;
}

/*Decrypt message using AES128 algorithm*/
uint8_t * decrypt(uint8_t *message)
{
	size_t message_len;
	static uint8_t decrypted_msg[512];

	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

	message_len = strlen(message);
	memcpy(decrypted_msg, message, message_len);

	/*Decrypt message*/
	AES_CBC_decrypt_buffer(&ctx, decrypted_msg, message_len);

	PRINTF("Decrypted Message: ");
	for(int i=0; i<message_len; i++)
	    {
		PRINTF("0x%02x,", decrypted_msg[i]);
		}
	PRINTF("\r\n");

	return decrypted_msg;
}

uint32_t calculate_crc32(uint8_t *message, size_t length, crc_config_t config, CRC_Type *base)
{
	uint32_t checksum32;

	InitCrc32(base, config);
	CRC_WriteData(base, (uint8_t *)&message[0], length-4);
	checksum32 = CRC_Get32bitResult(base);

	return checksum32;

}
