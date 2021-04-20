/*
 * FILE NAME: main.c                    			 COPYRIGHT (c) NXP 2016
 *                                                      All Rights Reserved
 * DESCRIPTION:
 * Default configurations: RUN mode, clock source = 48MHz Fast IRC,
 * 				Core Clk = System Clk = Bus Clk = 48 MHz, Flash Clk = 24 MHz
 * This example walks through the steps to delete all keys and reset the part
 * to the factory state.
 * As a reset, CSEc will be disabled, all keys will be reseted, FlexNVM will
 * reset to the Data Flash and FlexRAM will reset to the general RAM operation.
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

uint32_t dbg_challenge_out[4] = {0,0,0,0};

int main(void)
{
	uint16_t __attribute__((unused)) csec_error = 0; //1 means No Error

	csec_error = INIT_RNG(); /* Initialize the Random Number Generator before generating challenge */

    //To Erase all keys and reset the part to the factory state
    csec_error = DBG_CHAL(dbg_challenge_out); /* Generate the Challenge */
    csec_error = DBG_AUTH(dbg_challenge_out); /* Issue the Authorization */

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
