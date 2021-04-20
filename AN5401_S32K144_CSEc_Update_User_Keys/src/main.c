/*
 * FILE NAME: main.c                    			 COPYRIGHT (c) NXP 2016
 *                                                      All Rights Reserved
 * DESCRIPTION:
 * Default configurations: RUN mode, clock source = 48MHz Fast IRC,
 * 				Core Clk = System Clk = Bus Clk = 48 MHz, Flash Clk = 24 MHz
 *
 * First run example: "Configure_part_and_Load_keys" to enable CSEc and load
 * initial keys
 * This example walks through the steps to update already programmed Keys
 * with new keys.
 * 	Step 1: Authorizing Key = Master ECU Key or Key itself
 * 	Step 2: Increase counter
 * 	Step 3: Specify new key value
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
uint32_t NEW_KEY_1_VALUE[4] = {0xAF1256FD,0xBE89565C,0xF128ED6A, 0XAA5544FF};
uint32_t NEW_KEY_11_VALUE[4] = {0XC5D48AE6, 0x5500FFAA, 0x05EF698B, 0XB0E2A367};

uint8_t compare_results(uint32_t *M4_SW, uint32_t *M4_CSEc);

int main(void)
{
	uint16_t __attribute__((unused)) csec_error = 0; //1 means No Error
	uint8_t __attribute__((unused)) result = 0; //1 means No Error

	/* Update KEY_1 */
	/* Authorizing Key = MASER_ECU_KEY and counter value is increased to 2 */
    calculate_M1_to_M5(M1, M2, M3, M4, M5, MASTER_ECU_KEY_VALUE, NEW_KEY_1_VALUE, MASTER_ECU_KEY, KEY_1, 2, 0); /* Calculate M1 to M5 in Software */
    csec_error = LOAD_KEY(M4_out, M5_out, M1, M2, M3, KEY_1); /* Load the key using M1 to M3, returns M4 and M5 */
    result = compare_results(M4, M4_out); /* Compare M4 generated by SW with the M4_out returned by CSEc */

	/* Update KEY_11 */
	/* Authorizing Key = MASER_ECU_KEY and counter value is increased to 2 */
    calculate_M1_to_M5(M1, M2, M3, M4, M5, MASTER_ECU_KEY_VALUE, NEW_KEY_11_VALUE, MASTER_ECU_KEY, KEY_11, 2, 0b000100); /* Calculate M1 to M5 in Software */
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

//Check both SW generated and CSEc Generated M4
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
