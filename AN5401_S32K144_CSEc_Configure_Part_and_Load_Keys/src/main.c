/*
 * FILE NAME: main.c                    			 COPYRIGHT (c) NXP 2016
 *                                                      All Rights Reserved
 * DESCRIPTION:
 * Default configurations: RUN mode, clock source = 48MHz Fast IRC,
 * 				Core Clk = System Clk = Bus Clk = 48 MHz, Flash Clk = 24 MHz
 * This example enables the CSEc and walk through the steps to load the Keys
 * to the secure locations
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

uint32_t M1[4], M2[8], M3[4], M4[8], M5[4], M4_out[8], M5_out[4];

uint8_t compare_results(uint32_t *M4_SW, uint32_t *M4_CSEc);

int main(void)
{
	uint8_t __attribute__((unused)) flash_error = 0;
	uint16_t __attribute__((unused)) csec_error = 0; //1 means No Error
	uint8_t __attribute__((unused)) result = 0; //1 means No Error

	/* This configures part for 24 keys, SFE=0, load EERAM on reset, 4K EERAM and 64K EEPROM-backup */
	flash_error = configure_part_CSEc();
    csec_error = INIT_RNG();

//    /* Load RAM_KEY and export it */
//    csec_error = LOAD_RAM_KEY(RAM_KEY_VALUE);
//    csec_error = EXPORT_RAM_KEY(M1,M2,M3,M4,M5);

	/* Load MASTER_ECU_KEY */
    calculate_M1_to_M5(M1, M2, M3, M4, M5, BLANK_KEY_VALUE, MASTER_ECU_KEY_VALUE, MASTER_ECU_KEY, MASTER_ECU_KEY, 1, 0); /* Calculate M1 to M5 in Software */
    csec_error = LOAD_KEY(M4_out, M5_out, M1, M2, M3, MASTER_ECU_KEY); /* Load the key using SW calculated M1 to M3, and it returns M4 and M5 */
    result = compare_results(M4, M4_out); /* Compare M4 generated by SW with the M4_out returned by CSEc */

	/* Load KEY_1 */
    calculate_M1_to_M5(M1, M2, M3, M4, M5, MASTER_ECU_KEY_VALUE, KEY_1_VALUE, MASTER_ECU_KEY, KEY_1, 1, 0); /* Calculate M1 to M5 in Software, Authorizing Key = Master ECU Key */
    csec_error = LOAD_KEY(M4_out, M5_out, M1, M2, M3, KEY_1); /* Load the key using M1 to M3, returns M4 and M5 */
    result = compare_results(M4, M4_out); /* Compare M4 generated by SW with the M4_out returned by CSEc */

	/* Load KEY_11 */
    calculate_M1_to_M5(M1, M2, M3, M4, M5, MASTER_ECU_KEY_VALUE, KEY_11_VALUE, MASTER_ECU_KEY, KEY_11, 1, 0b000100); /* Calculate M1 to M5 in Software, Authorizing Key = Master ECU Key, Key Usage=1(for CMAC operations) */
    csec_error = LOAD_KEY(M4_out, M5_out, M1, M2, M3, KEY_11); /* Load the key using M1 to M3, returns M4 and M5 */
    result = compare_results(M4, M4_out); /* Compare M4 generated by SW with the M4_out returned by CSEc */

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

/* Check both SW generated and CSEc Generated M4 */
uint8_t compare_results(uint32_t *M4_SW, uint32_t *M4_CSEc)
{
	uint8_t success=1, i;

	for(i=0;i<8;i++)
	{
		if(M4_CSEc[i] != M4_SW[i])
		{
			success = 0;
			break;
		}
	}
	return success;
}
