/*
 * CSEc_functions.c
 *
 *  Created on: Oct 17, 2016
 *      Author: B55457
 */
#include "CSEc_macros.h"
//#include "CSEc_keys.h"

#define number_of_words(j_local, message_length) ((128*7*(j_local+1) > (message_length + 128))?((((message_length+128)-(128*7*j_local))/32) + 4):(32))

uint8_t flash_error_status = 0;
uint8_t first_iteration = 0;
uint16_t csec_error_bits,page_length;

uint32_t i,j,k;

uint32_t KEY_UPDATE_ENC_C[4] = {0x01015348, 0x45008000, 0x00000000, 0x000000B0};
uint32_t KEY_UPDATE_MAC_C[4] = {0x01025348, 0x45008000, 0x00000000, 0x000000B0};
uint32_t DEBUG_KEY_C[4] = 	   {0x01035348, 0x45008000, 0x00000000, 0x000000B0};


/* Enables CSEc by issuing the Program Partition Command, procedure: Figure 32-8 in RM, Configures for all 20 Keys */
uint8_t configure_part_CSEc(void)
{

    while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); /* Wait until any ongoing flash operation is completed */
    FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  /* Write 1 to clear error flags */

    FTFC->FCCOB[3] = 0x80; /* FCCOB0 = 0x80, program partition command */
    FTFC->FCCOB[2] = 0x03; /* FCCOB1 = 2b11, 20 keys */
    FTFC->FCCOB[1] = 0x00; /* FCCOB2 = 0x00, SFE = 0, VERIFY_ONLY attribute functionality disable */
    FTFC->FCCOB[0] = 0x00; /* FCCOB3 = 0x00, FlexRAM will be loaded with valid EEPROM data during reset sequence */
    FTFC->FCCOB[7] = 0x02; /* FCCOB4 = 0x02, 4k EEPROM Data Set Size */
    FTFC->FCCOB[6] = 0x04; /* FCCOB5 = 0x04, no data flash, 64k(all) EEPROM backup */

    FTFC->FSTAT = FTFC_FSTAT_CCIF_MASK; /* Start command execution by writing 1 to clear CCIF bit */

    while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); /* Wait until ongoing flash operation is completed */

    flash_error_status = FTFC->FSTAT; /* Read the flash status register for any Execution Error */

    return flash_error_status;
}

/* Get the UID */
uint16_t GET_UID(uint32_t *UID, uint32_t *UID_MAC)
{
	uint32_t challenge_in [4] = {0x12345678, 0x12345678, 0x12345678, 0x12345678};

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); /* Wait until any ongoing flash operation is completed */

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  /* Write 1 to clear error flags */

	for(i=4,j=0; i<8; i++,j++) // Write to Page1
		CSE_PRAM->RAMn[i].DATA_32= challenge_in[j]; /* Load the Challenge string */

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_GET_ID << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00); /*Write to Page0 Word0,  Value = 0x10000000,
	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	   No input Key */

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); /* Wait until ongoing CSEc operation is completed */

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; /*Read Page0 Word1, Error Bits field to check for any CSEc Execution error */

	for(i=8, j=0; i<12; i++, j++) //Read from Page2
		UID[j] = CSE_PRAM->RAMn[i].DATA_32; /* Retrieve the UID */

	for(i=12, j=0; i<16; i++, j++) //Read from Page3
		UID_MAC[j] = CSE_PRAM->RAMn[i].DATA_32; /* Retrieve the UID_MAC, calculated using the MASTER_ECU_KEY */

	return csec_error_bits;
}

/* Initialize Random Number Generator */
uint16_t INIT_RNG(void)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_INIT_RNG << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00); //Write to Page0 Word0,  Value = 0x0A000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

uint16_t GENERATE_RANDOM_NUMBER(uint32_t *random_number)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_RND << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00); //Write to Page0 Word0,  Value = 0x0C000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	for(i=4,j=0; i<8; i++,j++) //Read from Page1
		random_number[j] = CSE_PRAM->RAMn[i].DATA_32;

	return csec_error_bits;
}

/* Load the Plain Key in RAM_KEY slot */
uint16_t LOAD_RAM_KEY(uint32_t *key)
{

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	for(i=4,j=0; i<8; i++,j++) //Write to Page1
		CSE_PRAM->RAMn[i].DATA_32= key[j];

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_LOAD_PLAIN_KEY << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00); //Write to Page0 Word0,  Value = 0x08000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

/* Export RAM_KEY key in form of M1 to M5 and using EXPORT_RAM_KEY command */
uint16_t EXPORT_RAM_KEY(uint32_t *M1_out, uint32_t *M2_out, uint32_t *M3_out, uint32_t *M4_out, uint32_t *M5_out)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_EXPORT_RAM_KEY << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00); //Write to Page0 Word0,  Value = 0x09000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	for(i=4,j=0; i<8; i++,j++) //Read from Page1
		M1_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	for(i=8,j=0; i<16; i++,j++) //Read from Page2
		M2_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	for(i=16,j=0; i<20; i++,j++) //Read from Page3
		M3_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	for(i=20,j=0; i<28; i++,j++) //Read from Page4
		M4_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	for(i=28,j=0; i<32; i++,j++) //Read from Page5
		M5_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	return csec_error_bits;
}

/* Load Secret Keys (Except RAM_KEY) */
uint16_t LOAD_KEY(uint32_t *M4_out, uint32_t *M5_out, uint32_t *M1_in, uint32_t *M2_in, uint32_t *M3_in, uint8_t key_id)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	for(i=4,j=0; i<8; i++,j++) //Write to Page1
		CSE_PRAM->RAMn[i].DATA_32 = M1_in[j];

	for(i=8,j=0; i<16; i++,j++) //Write to Page2-3
		CSE_PRAM->RAMn[i].DATA_32 = M2_in[j];

	for(i=16,j=0; i<20; i++,j++) //Write to Page4
		CSE_PRAM->RAMn[i].DATA_32 = M3_in[j];

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_LOAD_KEY << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id;// Write to Page0 Word0, Value = 0x07000000 | key_id

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	for(i=20,j=0; i<28; i++,j++) //Read from Page5-6
		M4_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	for(i=28,j=0; i<32; i++,j++) //Read from Page7
		M5_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	return csec_error_bits;
}

/* Encode the date using ECB:Electronic Code Book Mode
 * For simplicity this function is developed for up to first 7 pages of data (112 bytes)
 */
uint16_t ENC_ECB(uint32_t *cipher_text, uint32_t *plain_text, uint8_t key_id, uint16_t page_length)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	for(i=4,j=0; i<(page_length*4 + 4); i++,j++) // Fill all pages, word by word
		CSE_PRAM->RAMn[i].DATA_32 = plain_text[j];


	CSE_PRAM->RAMn[3].DATA_32= page_length; // Write to Page0 Word3, Value = Number of Pages

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32=(CMD_ENC_ECB << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; //Write to Page0 Word0, Value = 0x02000000 | key_id

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Write to Page0 Word1

	for(i=4,j=0; i<(page_length*4 + 4); i++,j++)
		cipher_text[j] = CSE_PRAM->RAMn[i].DATA_32; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

/* Encode the date using CBC:Cipher Block Chaining Mode
 * For simplicity this function is developed for up to first 6 pages of data(96 bytes)
 */
uint16_t ENC_CBC(uint32_t *cipher_text, uint32_t *IV, uint8_t key_id, uint32_t *plain_text, uint16_t page_length)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	for(i=4,j=0; i<8; i++,j++) //Write to Page1
		CSE_PRAM->RAMn[i].DATA_32 = IV[j];

	for(i=8,j=0; i<(page_length*4+8); i++,j++) // Fill all other pages, word by word
		CSE_PRAM->RAMn[i].DATA_32 = plain_text[j];

	CSE_PRAM->RAMn[3].DATA_32= page_length; // Write to Page0 Word3, Value = Number of Pages

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32=(CMD_ENC_CBC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; // Write to Page0 Word0, Value = 0x02000000 | key_id

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Write to Page0 Word1

	for(i=8,j=0; i<(page_length*4+8); i++,j++)
		cipher_text[j] = CSE_PRAM->RAMn[i].DATA_32; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

/* Decode the cipher data using CBC:Cipher Block Chaining Mode
 * For simplicity this function is developed for up to first 6 pages of data
 */
uint16_t DEC_CBC(uint32_t *deciphered_text, uint32_t *IV, uint8_t key_id, uint32_t *cipher_text, uint16_t page_length)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	for(i=4,j=0; i<8; i++,j++) //Write to Page1
		CSE_PRAM->RAMn[i].DATA_32 = IV[j];

	for(i=8,j=0; i<(page_length*4+8); i++,j++) // Fill all other pages, word by word
		CSE_PRAM->RAMn[i].DATA_32 = cipher_text[j];

	CSE_PRAM->RAMn[3].DATA_32= page_length; // Write to Page0 Word3, Value = Number of Pages

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32=(CMD_DEC_CBC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; // Write to Page0 Word0, Value = 0x04000000 | key_id

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Write to Page0 Word1

	for(i=8,j=0; i<(page_length*4+8); i++,j++)
		deciphered_text[j] = CSE_PRAM->RAMn[i].DATA_32; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}


/* Decode the cipher data using CBC:Cipher Block Chaining Mode
 * For simplicity this function is developed for up to first 6 pages of data
 */
uint16_t DEC_CBC_LONG(uint32_t *deciphered_text, uint32_t *IV, uint8_t key_id, uint32_t *cipher_text, uint16_t page_length_total)
{

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	while(page_length_total>0)
	{

		if(first_iteration==0)
		{
			if(page_length_total>6)	//Limit the number of phrases to 6 on the first iteration
			{
				page_length=6;
				page_length_total-=6;	//Decrease the number of phrases left to decrypt
			}
			else
			{
				page_length=page_length_total;
				page_length_total=0;
			}

			for(i=4,j=0; i<8; i++,j++) //Write to Page1
				CSE_PRAM->RAMn[i].DATA_32 = IV[j];

			for(i=8,j=0; i<(page_length*4+8); i++,j++) // Fill all other pages, word by word
				CSE_PRAM->RAMn[i].DATA_32 = cipher_text[j];
			CSE_PRAM->RAMn[3].DATA_32= page_length; // Write to Page0 Word3, Value = Number of Pages

			/* Start command by wring Header */
			CSE_PRAM->RAMn[0].DATA_32=(CMD_DEC_CBC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; // Write to Page0 Word0, Value = 0x04000000 | key_id

			while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

			csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Write to Page0 Word1

			for(i=8,j=0; i<(page_length*4+8); i++,j++)
				deciphered_text[j] = CSE_PRAM->RAMn[i].DATA_32; //Read Page0 Word1, Error Bits

			first_iteration=1;
			k=24;	// index word for the next ongoing operation (6 phrases * 4 words)
		}
		else	//Indicate is an ongoing operation
		{
			if(page_length_total>7)	//Limit the number of phrases to 7 on further iterations
			{
				page_length=7;
				page_length_total-=7;	//Decrease the number of phrases left to decrypt
			}
			else
			{
				page_length=page_length_total;
				page_length_total=0;
			}
			for(i=4,j=k; i<(page_length*4+4); i++,j++) // Fill all other pages, word by word
				CSE_PRAM->RAMn[i].DATA_32 = cipher_text[j];

			CSE_PRAM->RAMn[3].DATA_32= page_length; // Write to Page0 Word3, Value = Number of Pages

			/* Start command by wring Header */
			CSE_PRAM->RAMn[0].DATA_32=(CMD_DEC_CBC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id; // Write to Page0 Word0, Value = 0x04000000 | key_id

			while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

			csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Write to Page0 Word1

			for(i=4,j=k; i<(page_length*4+4); i++,j++)
				deciphered_text[j] = CSE_PRAM->RAMn[i].DATA_32; //Read Page0 Word1, Error Bits
			k+=(page_length*4);				//VERIFY THIS POINT J=K and when the image is completed
		}
	}

	return csec_error_bits;
}
/* Generate the MAC for the data
 * Data copy format feature is used
 * No data size limit
 *  */
uint16_t CMAC(uint32_t *cmac, uint32_t *data, uint8_t key_id, uint32_t message_length)
{

	uint32_t j_local = 0;

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	while(j_local <= message_length/(128*7))
	{

		for(i=4; i<32; i++)
			CSE_PRAM->RAMn[i].DATA_32 = data[(i-4) + j_local*28];

		CSE_PRAM->RAMn[3].DATA_32= message_length; // Write to Page0 Word3

		/* Start command by wring Header */
		if(j_local==0)
			CSE_PRAM->RAMn[0].DATA_32=(CMD_GENERATE_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; // Write to Page0 Word0, Value = 0x05000000 | KEY;
		else
			CSE_PRAM->RAMn[0].DATA_32=(CMD_GENERATE_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id; // Write to Page0 Word0, Value = 0x05000100 | KEY;

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits
		if(csec_error_bits != 1)
			break;
	j_local++;
	}

	for(i=8; i<12; i++)
		cmac[i-8] = CSE_PRAM->RAMn[i].DATA_32;


	return csec_error_bits;
}

/* Generate the BOOT_MAC for the application to be protected
 * Data copy format feature is used. For this function, message_length should be in multiple of 128 bits and greater than 0.
 * 512kB data only
 * Remember to put message length on PAGE[1] in the first loop. So now message_length  = message_length + 128
 *  */
uint16_t MAC_SECURE_BOOT(uint32_t *cmac, uint32_t *data, uint8_t key_id, uint32_t message_length)
{

	uint32_t j_local = 0;

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	uint32_t number_of_pram_load_cyc = ((message_length+128)/(128*7)) + (((message_length+128)%(128*7))?(1):(0));

	while(j_local < number_of_pram_load_cyc)
	{

		if(j_local == 0)
		{
			CSE_PRAM->RAMn[4].DATA_32 = 0;
			CSE_PRAM->RAMn[5].DATA_32 = 0;
			CSE_PRAM->RAMn[6].DATA_32 = 0;
			CSE_PRAM->RAMn[7].DATA_32 = message_length;
			for(i=8; i<number_of_words(j_local, message_length); i++)
				CSE_PRAM->RAMn[i].DATA_32 = data[i-8];
		}
		else if(j_local == 1)
		{
			for(i=4; i<number_of_words(j_local, message_length); i++)
				CSE_PRAM->RAMn[i].DATA_32 = data[(i-4) + j_local*24];
		}
		else
		{
			for(i=4; i<number_of_words(j_local, message_length); i++)
				CSE_PRAM->RAMn[i].DATA_32 = data[(i-4) + j_local*28 - 4];
		}

		CSE_PRAM->RAMn[3].DATA_32= message_length+128; // Write to Page0 Word3

		/* Start command by wring Header */
		if(j_local==0)
			CSE_PRAM->RAMn[0].DATA_32=(CMD_GENERATE_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; // Write to Page0 Word0, Value = 0x05000000 | KEY;
		else
			CSE_PRAM->RAMn[0].DATA_32=(CMD_GENERATE_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id; // Write to Page0 Word0, Value = 0x05000100 | KEY;

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits
		if(csec_error_bits != 1)
			break;
	j_local++;
	}

	for(i=8; i<12; i++)
		cmac[i-8] = CSE_PRAM->RAMn[i].DATA_32;


	return csec_error_bits;
}

/* Generate the MAC for the data
 * Data copy format feature is used
 * No data size limit
 *  */
uint16_t CMAC_VERIFY(uint16_t *verification_status, uint32_t *data_and_cmac, uint8_t key_id, uint32_t message_length)
{
	uint8_t j_local = 0;
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	while(j_local <= message_length/(128*7))
	{

		for(i=4; i<32; i++)
			CSE_PRAM->RAMn[i].DATA_32 = data_and_cmac[(i-4) + j_local*28];

		CSE_PRAM->RAMn[3].DATA_32= message_length; // Write to Page0 Word3
		CSE_PRAM->RAMn[2].DATA_32=128 << 16; //Write to MSBs of Page0 Word2

		/* Start command by wring Header */
		if(j_local==0)
			CSE_PRAM->RAMn[0].DATA_32=(CMD_VERIFY_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | key_id; // Write to Page0 Word0, Value = 0x05000000 | KEY;
		else
			CSE_PRAM->RAMn[0].DATA_32=(CMD_VERIFY_MAC << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_SUBSEQUENT << 8) | key_id; // Write to Page0 Word0, Value = 0x05000100 | KEY;

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits
	j_local++;
	}

	*verification_status = CSE_PRAM->RAMn[5].DATA_32 >> 16; //Read Page1 Word1, Error Bits

	return csec_error_bits;
}
/* Enable Secure boot and Specify Secure Boot method and Code Size */
uint16_t BOOT_DEFINE(uint32_t boot_size, uint8_t boot_flavor)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	CSE_PRAM->RAMn[7].DATA_32 = boot_size; //Write to Page1 Word3
	CSE_PRAM->RAMn[6].DATA_32 = boot_flavor; //Write to Page1 Word2

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32 = (CMD_BOOT_DEFINE << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | 0x00; // Write to Page0 Word0, Value = 0x11000000 Start command by wring value

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

/* This command may be used when checking additional blocks of information after the autonomous secure boot finishes */
uint16_t BOOT_FAILURE(void)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_BOOT_FAILURE << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | 0x00; // Write to Page0 Word0, Value = 0x0E000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

/* BOOT_OK comand is followed by the successful secure boot to mark that secure boot is finished */
uint16_t BOOT_OK(void)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32 = (CMD_BOOT_OK << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | 0x00; // Write to Page0 Word0, Value = 0x0F000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	return csec_error_bits;
}

/* Calculate KDF (as specified in SHE) for various operations */
uint16_t KDF(uint32_t *K_out, uint32_t *authorizing_key, uint32_t *constant)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  /* Write 1 to clear error flags */

	for(i=4,j=0; i<8; i++,j++)//Write to Page1
		CSE_PRAM->RAMn[i].DATA_32 = authorizing_key[j];

	for(i=8,j=0; i<12; i++,j++)//Write to Page2
		CSE_PRAM->RAMn[i].DATA_32 = constant[j];

	CSE_PRAM->RAMn[3].DATA_32= 0x00000002; // Write to Page0 Word3, Value= Number of Pages

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_MP_COMPRESS << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | (0x00); // Write to Page0 Word0, Value = 0x16000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >>16; //Read Page0 Word1, Error Bits

	for(i=4,j=0; i<8; i++,j++)
		K_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	return csec_error_bits;
}

/* Issue Debug challenge command */
uint16_t DBG_CHAL(uint32_t *dbg_challenge_out)
{
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);// Write 1 to clear error flags */

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_DBG_CHAL << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | 0x00; //Write to Page0 Word0, Value = 0x12000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	for(i=4,j=0;i<8;i++,j++)
		dbg_challenge_out[j] = CSE_PRAM->RAMn[i].DATA_32;

	return csec_error_bits;
}

/* Issue Debug Authorization command */
uint16_t DBG_AUTH(uint32_t *dbg_challenge_out)
{
	uint32_t K_out[4];

	uint32_t UID[4] = {0,0,0,0};
	uint32_t UID_MAC[4] = {0,0,0,0};
	uint32_t authorization[4];
	uint32_t MASTER_ECU_KEY_VALUE[4] =	{0xD275F12C, 0xA863A7B5, 0xF933DF92, 0x6498FB4D}; //MASTER_ECU_KEY


	//First Calculate the Authorization
    csec_error_bits = GET_UID(UID, UID_MAC);

	csec_error_bits = KDF(K_out, MASTER_ECU_KEY_VALUE, DEBUG_KEY_C);

	csec_error_bits = LOAD_RAM_KEY(K_out);

	uint32_t DATA[8];
	for(i=0; i<4; i++)
		DATA[i] = dbg_challenge_out[i];
	for(;i<8;i++)
		DATA[i] = UID[i-4];

	csec_error_bits = CMAC(authorization, DATA, RAM_KEY, 248);

	//Now actually issue authorization command
	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != FTFC_FSTAT_CCIF_MASK); //Check for the ongoing FLASH command

	FTFC->FSTAT = (FTFC_FSTAT_FPVIOL_MASK | FTFC_FSTAT_ACCERR_MASK);  // Write 1 to clear error flags

	for(i=4,j=0; i<8; i++,j++) //Write to Page1
		CSE_PRAM->RAMn[i].DATA_32 = authorization[j];

	/* Start command by wring Header */
	CSE_PRAM->RAMn[0].DATA_32= (CMD_DBG_AUTH << 24) | (CMD_FORMAT_COPY << 16) | (CALL_SEQ_FIRST << 8) | 0x00; // Write to Page0 Word0, Value = 0x13000000

	while((FTFC->FSTAT & FTFC_FSTAT_CCIF_MASK) != 0x80); //Check for the ongoing FLASH command

	csec_error_bits = CSE_PRAM->RAMn[1].DATA_32 >> 16; //Read Page0 Word1, Error Bits

	return csec_error_bits;

}

/* Calculate M1 to M3 for Any key, Assuming keys are wild card protected. So Using UID=0. */
void calculate_M1_to_M5(uint32_t *M1_out, uint32_t *M2_out, uint32_t *M3_out, uint32_t *M4_out, uint32_t *M5_out,
						uint32_t *authorizing_key, uint32_t *new_key, uint8_t auth_key_id, uint8_t key_id, uint32_t counter, uint32_t attribute_flags)
{
	uint32_t UID[4] = {0,0,0,0};
	uint32_t UID_MAC[4] = {0,0,0,0};
	uint32_t M4_star[4] = {0,0,0,0};
	uint32_t K_out[4] = {0,0,0,0};
	uint32_t all_zero[4] = {0, 0, 0, 0};
	uint8_t i = 0;

	key_id = key_id & 0x0F; //For keys in the other bank, don't consider the KBS bit in M1 & M4 calculation. Follow traditional SHE specs

//    csec_error_bits = GET_UID(UID, UID_MAC); //Assuming the Keys are wild card protected. So Using UID=0.

    //Calculate M1
	M1_out[0] = UID[0];
	M1_out[1] = UID[1];
	M1_out[2] = UID[2];
	M1_out[3] = UID[3] | (key_id<<4) | auth_key_id; //Using key it self for the authorizaion


	//Calculate M2
	//First, calculate K1 and load K1 into RAM
	csec_error_bits = KDF(K_out, authorizing_key, KEY_UPDATE_ENC_C);
	csec_error_bits = LOAD_RAM_KEY(K_out);
	//Second, concate the input data
	uint32_t PLAIN_TEXT[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	PLAIN_TEXT[0] = (counter << 4) | (attribute_flags >> 2);
	PLAIN_TEXT[1] = attribute_flags << 30;
	PLAIN_TEXT[4] = new_key[0];
	PLAIN_TEXT[5] = new_key[1];
	PLAIN_TEXT[6] = new_key[2];
	PLAIN_TEXT[7] = new_key[3];

	csec_error_bits = ENC_CBC(M2_out, all_zero, RAM_KEY, PLAIN_TEXT, 2);

	//Calculate M3
	//First, calculate K2 load it as RAM_KEY
	csec_error_bits = KDF(K_out, authorizing_key, KEY_UPDATE_MAC_C);
	csec_error_bits = LOAD_RAM_KEY(K_out);

	uint32_t DATA[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
	for(i=0; i<4; i++)
		DATA[i] = M1_out[i];
	for(; i<12; i++)
		DATA[i] = M2_out[i-4];

	csec_error_bits = CMAC(M3_out, DATA, RAM_KEY, 384);

	//Calculate M4
	csec_error_bits = GET_UID(UID, UID_MAC);

	//First, calculate K3 load it as RAM_KEY
	csec_error_bits = KDF(K_out, new_key, KEY_UPDATE_ENC_C);
	csec_error_bits = LOAD_RAM_KEY(K_out);

	M4_star[0] = (counter << 4) | 0x8;
	M4_star[1] = 0;
	M4_star[2] = 0;
	M4_star[3] = 0;

	csec_error_bits = ENC_ECB(M4_star, M4_star, RAM_KEY, 2);

	for(i=0;i<4;i++)
		M4_out[i] = UID[i];
	M4_out[3] = (M4_out[3] & 0xFFFFFF00) | (key_id << 4) | auth_key_id;
	for(i=0;i<4;i++)
			M4_out[i+4] = M4_star[i];

	//Calculate M5
	csec_error_bits = KDF(K_out, new_key, KEY_UPDATE_MAC_C);
	csec_error_bits = LOAD_RAM_KEY(K_out);
	csec_error_bits = CMAC(M5_out, M4_out, RAM_KEY, 256);

}
