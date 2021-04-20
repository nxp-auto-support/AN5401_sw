/*
 * CSEc_functions.h
 *
 *  Created on: Oct 17, 2016
 *      Author: B55457
 */

#ifndef CSEC_FUNCTIONS_H_
#define CSEC_FUNCTIONS_H_

uint8_t configure_part_CSEc(void);
uint16_t GET_UID(uint32_t *UID, uint32_t *UID_MAC);
uint16_t INIT_RNG(void);
uint16_t GENERATE_RANDOM_NUMBER(uint32_t *random_number);
uint16_t LOAD_RAM_KEY(uint32_t *key);
uint16_t EXPORT_RAM_KEY(uint32_t *M1_out, uint32_t *M2_out, uint32_t *M3_out, uint32_t *M4_out, uint32_t *M5_out);
void calculate_M1_to_M5(uint32_t *M1_out, uint32_t *M2_out, uint32_t *M3_out, uint32_t *M4_out, uint32_t *M5_out,
						uint32_t *authorizing_key, uint32_t *new_key, uint8_t auth_key_id, uint8_t key_id, uint32_t counter, uint32_t attribute_flags);
uint16_t KDF(uint32_t *K_out, uint32_t *authorizing_key, uint32_t *constant);
uint16_t ENC_ECB(uint32_t *cipher_text, uint32_t *plain_text, uint8_t key_id, uint16_t page_length);
uint16_t ENC_CBC(uint32_t *cipher_text, uint32_t *IV, uint8_t key_id, uint32_t *plain_text, uint16_t page_length);
uint16_t DEC_CBC(uint32_t *deciphered_text, uint32_t *IV, uint8_t key_id, uint32_t *cipher_text, uint16_t page_length);
uint16_t CMAC(uint32_t *cmac, uint32_t *data, uint8_t key_id, uint32_t message_length);
uint16_t MAC_SECURE_BOOT(uint32_t *cmac, uint32_t *data, uint8_t key_id, uint32_t message_length);
uint16_t CMAC_VERIFY(uint16_t *verification_status, uint32_t *data_and_cmac, uint8_t key_id, uint32_t message_length);
uint16_t BOOT_FAILURE(void);
uint16_t BOOT_OK(void);
uint16_t BOOT_DEFINE(uint32_t boot_size, uint8_t boot_flavor);
uint16_t LOAD_KEY(uint32_t *M4_out, uint32_t *M5_out, uint32_t *M1_in, uint32_t *M2_in, uint32_t *M3_in, uint8_t key_id);
uint16_t DBG_CHAL(uint32_t *dbg_challenge_out);
uint16_t DBG_AUTH(uint32_t *dbg_challenge_out);


#endif /* CSEC_FUNCTIONS_H_ */
