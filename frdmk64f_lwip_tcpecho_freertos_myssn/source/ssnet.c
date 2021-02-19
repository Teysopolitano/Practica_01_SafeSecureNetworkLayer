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

///* CRC data */
//CRC_Type *base = CRC0;
//uint32_t checksum32;


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
	static uint8_t decrypted_msg[512];

//	p_dec_msg = &decrypted_msg;

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

uint32_t calculate_crc32(uint8_t *message, size_t length)
{
	/* CRC data */
	CRC_Type *base = CRC0;
	uint32_t checksum32;

	InitCrc32(base, 0xFFFFFFFFU);
	CRC_WriteData(base, (uint8_t *)&message[0], length-4);
	checksum32 = CRC_Get32bitResult(base);

	return checksum32;

}

void tcpecho_server_ssnet_thread(void *arg)
{
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(arg);

  /* CRC data */
//  CRC_Type *base = CRC0;
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
            checksum32 = calculate_crc32(p_message, len);
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
        		checksum32 = calculate_crc32(p_enc_msg, len);
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

