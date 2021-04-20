/*
 * CSEc_macros.h
 *
 *  Created on: Oct 14, 2016
 *      Author: B55457
 */

#ifndef CSEC_MACROS_H_
#define CSEC_MACROS_H_

#include <S32K144.h>

typedef enum{
    CMD_ENC_ECB = 0x1U,
    CMD_ENC_CBC,
    CMD_DEC_ECB,
    CMD_DEC_CBC,
    CMD_GENERATE_MAC,
    CMD_VERIFY_MAC,
    CMD_LOAD_KEY,
    CMD_LOAD_PLAIN_KEY,
    CMD_EXPORT_RAM_KEY,
    CMD_INIT_RNG,
    CMD_EXTEND_SEED,
    CMD_RND,
    CMD_RESERVED_1,
    CMD_BOOT_FAILURE,
    CMD_BOOT_OK,
    CMD_GET_ID,
    CMD_BOOT_DEFINE,
    CMD_DBG_CHAL,
    CMD_DBG_AUTH,
    CMD_RESERVED_2,
    CMD_RESERVED_3,
    CMD_MP_COMPRESS
}csec_cmd_t;

typedef enum {
    CMD_FORMAT_COPY = 0x0U,
    CMD_FORMAT_ADDR
} csec_cmd_format_t;

typedef enum {
    CALL_SEQ_FIRST,
    CALL_SEQ_SUBSEQUENT
} csec_call_sequence_t;

typedef enum {
    SECRET_KEY = 0x0U,
    MASTER_ECU_KEY,
    BOOT_MAC_KEY,
    BOOT_MAC,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_10,
    RAM_KEY = 0xFU,
    KEY_11 = 0x14U,
    KEY_12,
    KEY_13,
    KEY_14,
    KEY_15,
    KEY_16,
    KEY_17,
    KEY_18,
    KEY_19,
    KEY_20,
    KEY_21
} csec_key_id_t;

typedef enum {
    NO_ERROR            = 0x1U,
    SEQUENCE_ERROR      = 0x2U,
    KEY_NOT_AVAILABLE   = 0x4U,
    KEY_INVALID         = 0x8U,
    KEY_EMPTY           = 0x10U,
    NO_SECURE_BOOT      = 0x20U,
    KEY_WRITE_PROTECTED = 0x40U,
    KEY_UPDATE_ERROR    = 0x80U,
    RNG_SEED            = 0x100U,
    NO_DEBUGGING        = 0x200U,
    MEMORY_FAILURE      = 0x400U,
    GENERAL_ERROR       = 0x800U,
    BUSY                = 0x1000U //Need to Check
} csec_error_code_t;

#endif /* CSEC_MACROS_H_ */
