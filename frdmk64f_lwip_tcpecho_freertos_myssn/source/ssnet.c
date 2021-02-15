/*
 * ssnet.c
 *
 * Safe and Secure Network layer implementation
 *
 *  Created on: 13 feb. 2021
 *      Author: Luis Teyssier
 */

#include "ssnet.h"

#define SERVER_PORT 10000
#define SOCKET_TIMEOUT = 20


uint8_t test_string[] = {"01234567890123456789"};

/*AES variables*/
uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
struct AES_ctx ctx;

/* CRC data */
CRC_Type *base = CRC0;
uint32_t checksum32;


/*Encrypt message using AES128 algorithm*/
uint8_t encrypt(uint8_t *message)
{
	size_t message_len, padded_len;
	uint8_t padded_msg[512] = {0};

	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

	/* To encrypt an array its lenght must be a multiple of 16 so we add zeros */
	message_len = strlen(message);
	padded_len = message_len + (16 - (message_len%16) );
	memcpy(padded_msg, message, message_len);

	AES_CBC_encrypt_buffer(&ctx, padded_msg, padded_len);

	return padded_msg;
}


uint8_t decrypt(uint8_t *message)
{
	size_t message_len;
	uint8_t decrypted_msg[512];

	message_len = strlen(message);
	memcpy(decrypted_msg, message, message_len);

	AES_CBC_decrypt_buffer(&ctx, decrypted_msg, message_len);

	return decrypted_msg;
}

