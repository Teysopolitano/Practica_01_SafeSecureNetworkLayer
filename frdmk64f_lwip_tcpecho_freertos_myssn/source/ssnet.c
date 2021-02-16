/*
 * ssnet.c
 *
 * Safe and Secure Network layer implementation
 *
 *  Created on: 13 feb. 2021
 *      Author: Luis Teyssier
 */

#include "ssnet.h"


//#if LWIP_NETCONN

/*AES variables*/
uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
struct AES_ctx ctx;

/* CRC data */
CRC_Type *base = CRC0;
static uint32_t checksum32;

/*!
 * @brief Init for CRC-32.
 * @details Init CRC peripheral module for CRC-32 protocol.
 *          width=32 poly=0x04c11db7 init=0xffffffff refin=true refout=true xorout=0xffffffff check=0xcbf43926
 *          name="CRC-32"
 *          http://reveng.sourceforge.net/crc-catalogue/
 */
void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

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
	static uint8_t decrypted_msg[512]= {0};

	/* Init the AES context structure */
	AES_init_ctx_iv(&ctx, key, iv);

	message_len = strlen(message);
	memcpy(decrypted_msg, message, message_len);

    /*Decrypt message*/
	AES_CBC_decrypt_buffer(&ctx, decrypted_msg, message_len);

	PRINTF("Decrypted Message: ");
	for(int i=0; i<message_len; i++) {
		PRINTF("0x%02x,", decrypted_msg[i]);
		}
	PRINTF("\r\n");

	return decrypted_msg;
}

uint32_t calculate_crc32(uint8_t *message)
{

	size_t padded_len;

	padded_len = strlen(message);

	InitCrc32(base, 0xFFFFFFFFU);
	CRC_WriteData(base, (uint8_t *)&message[0], padded_len);
	checksum32 = CRC_Get32bitResult(base);

	return checksum32;

}

//void tcpecho_ssnet_thread(void *arg)
//{
//  struct netconn *conn, *newconn;
//  err_t err;
//  LWIP_UNUSED_ARG(arg);
//
//  size_t test_string_len, padded_len;
//  uint8_t *enc_msg;
//  uint8_t *dec_msg;
//
//  /* CRC data */
//  CRC_Type *base = CRC0;
//  uint32_t checksum32;
//
//  /* Create a new connection identifier. */
//  /* Bind connection to well known port number 7. */
//#if LWIP_IPV6
//  conn = netconn_new(NETCONN_TCP_IPV6);
//  netconn_bind(conn, IP6_ADDR_ANY, 7);
//#else /* LWIP_IPV6 */
//  conn = netconn_new(NETCONN_TCP);
//  netconn_bind(conn, IP_ADDR_ANY, 7);
//#endif /* LWIP_IPV6 */
//  LWIP_ERROR("tcpecho: invalid conn", (conn != NULL), return;);
//
//  /* Tell connection to go into listening mode. */
//  netconn_listen(conn);
//
//  while (1) {
//
//    /* Grab new connection. */
//    err = netconn_accept(conn, &newconn);
//    /*printf("accepted new connection %p\n", newconn);*/
//    /* Process the new connection. */
//    if (err == ERR_OK) {
//      struct netbuf *buf;
//      void *data;
//      u16_t len;
//
//      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
//        /*printf("Received\n");*/
//        do {
//
//        	netbuf_data(buf, &data, &len);
//
////        	/*Check CRC32*/
////        	InitCrc32(base, 0xFFFFFFFFU);
////        	CRC_WriteData(base, (uint8_t *)&padded_msg[0], padded_len);
////        	checksum32 = CRC_Get32bitResult(base);
////        	/*Decrypt message*/
////		    decrypted_message =  decrypt(uint8_t *message);
//
//
//
//             err = netconn_write(newconn, data, len, NETCONN_COPY);
//#if 0
//            if (err != ERR_OK) {
//              printf("tcpecho: netconn_write: error \"%s\"\n", lwip_strerr(err));
//            }
//#endif
//        } while (netbuf_next(buf) >= 0);
//        netbuf_delete(buf);
//      }
//      /*printf("Got EOF, looping\n");*/
//      /* Close connection and discard connection identifier. */
//      netconn_close(newconn);
//      netconn_delete(newconn);
//    }
//  }
//}
///*-----------------------------------------------------------------------------------*/
//
