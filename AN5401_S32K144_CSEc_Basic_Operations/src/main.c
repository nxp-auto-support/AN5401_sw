/*
 * FILE NAME: main.c                    			 COPYRIGHT (c) NXP 2016
 *                                                      All Rights Reserved
 * DESCRIPTION:
 * Default configurations: RUN mode, clock source = 48MHz Fast IRC,
 * 				Core Clk = System Clk = Bus Clk = 48 MHz, Flash Clk = 24 MHz
 * This example walks through following basic operations
 * 1. Random Number Generation
 * 2. UID Retrieval
 * 3. AES-128 Encryption and Decryption
 * 4. CMEC Generation and verification
 *
 * Note: Debug/Run from RAM as program updates the system or secure flash
 *
 * PLATFORM: S32K144 MB and DC with S32DS IDE and PnE Multilink FX Debugger
 */
/*****************************************************************************/
/* REV      AUTHOR        DATE        DESCRIPTION OF CHANGE                  */
/* ---   -----------    ----------    ---------------------                  */
/* 1.0	  K Shah        25 Oct 2016  Initial Version                         */
/*****************************************************************************/


#include "S32K144.h" /* include peripheral declarations S32K144 */
#include "CSEc_functions.h"
#include "CSEc_macros.h"
#include "CSEc_keys.h"
#include "NXP_logo.h"



int main(void)
{
	uint8_t __attribute__((unused)) flash_error = 0;
	uint16_t __attribute__((unused)) csec_error = 0; //1 means No Error
	uint8_t __attribute__((unused)) result = 0; //1 means No Error
	uint8_t __attribute__((unused)) success=1; //1 means Pass
	uint8_t __attribute__((unused)) i;
	uint16_t verification_status;
	uint16_t NXP_logo_size, num_pages,pagecnt;
	static uint8_t wordcnt;

	/* Initialize the Random Number Generator and Generate Random Number */
	uint32_t random_number[4];
	csec_error = INIT_RNG(); //Randum number generator must be initialize before generating random number
    csec_error = GENERATE_RANDOM_NUMBER(random_number);

    /* Retrieve the UID */
	uint32_t UID[4], UID_MAC[4];
    csec_error = GET_UID(UID, UID_MAC);

    /* AES-128 CBC mode Encryption and Decryption */
    uint32_t encrypted_text[4], decrypted_text[4], decrypted_image[13000];
    uint32_t IV[4] = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210 };
    uint32_t plain_text[8] = {559869,1232547,5236547,633458};

    csec_error = ENC_CBC(encrypted_text, IV, KEY_1, plain_text, 1);
    csec_error = DEC_CBC(decrypted_text, IV, KEY_1, encrypted_text, 1);

	for(i=0;i<4;i++) //Compare decrypted text with plain text
	{
		if(plain_text[i] != decrypted_text[i])
		{
			success = 0; //Fail
			break;
		}
	}

	/* AES-128 CBC Encryption and Decryption for large data */
	NXP_logo_size= sizeof(NXP_logo_cipher);
	num_pages= NXP_logo_size/16;

	csec_error = DEC_CBC_LONG(decrypted_image, IV, KEY_1, NXP_logo_cipher, num_pages);



	/* CMAC generation and verification */
	uint32_t data_and_cmac[8], cmac[4];

	//Generate CMAC for the plain text using key-1
	csec_error = CMAC(cmac, plain_text, KEY_11, 128);

	//Append plain text and cmac
	for(i=0; i<4; i++)
		data_and_cmac[i] = plain_text[i];
	for(i=4; i<8; i++)
		data_and_cmac[i] = cmac[i-4];

	// Run MAC verify command on above concated array
	csec_error = CMAC_VERIFY(&verification_status, data_and_cmac, KEY_11, 128);
	if(verification_status == 0)
		success=1; //Verification Passed
	else
		success=0; //Verification Fail


	while(1);

    /* to avoid the warning message for GHS and IAR: statement is unreachable*/
#if defined (__ghs__)
#pragma ghs nowarning 111
#endif
#if defined (__ICCARM__)
#pragma diag_suppress=Pe111
#endif
	return 0;
}
