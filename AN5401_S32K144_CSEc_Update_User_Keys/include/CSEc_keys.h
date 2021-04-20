/*
 * CSEc_keys.h
 *
 *  Created on: Oct 19, 2016
 *      Author: B55457
 */

#ifndef CSEC_KEYS_H_
#define CSEC_KEYS_H_

uint32_t BLANK_KEY_VALUE[4] = 		{0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF}; //When key value is not written it is all FFs
uint32_t MASTER_ECU_KEY_VALUE[4] =	{0xD275F12C, 0xA863A7B5, 0xF933DF92, 0x6498FB4D}; //MASTER_ECU_KEY
uint32_t BOOT_MAC_KEY_VALUE[4] =	{0x12340000, 0x00000000, 0x00000000, 0x00005678}; //BOOT_MAC_KEY
uint32_t KEY_1_VALUE[4] =			{0x2FF8B03C, 0x5C540546, 0x5A9C94BD, 0x2D863279}; //KEY_1
uint32_t KEY_11_VALUE[4] =			{0x85852FF8, 0xE7860C89, 0xB3AB9D63, 0xB8D6288F}; //KEY_11
uint32_t RAM_KEY_VALUE[4] =			{0x68B674CB, 0x8198A250, 0x3A285100, 0xF4DDC40A}; //RAM_KEY

#endif /* CSEC_KEYS_H_ */