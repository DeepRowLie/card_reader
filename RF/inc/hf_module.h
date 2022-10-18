#ifndef __HF_MODULE_H
#define __HF_MODULE_H

#include "main.h"

#define UID_LEN_MAX 7
#define BLOCK_ID    4
#define BLOCK_LEN   0x10
#define KEY_LEN     6

typedef struct
{
    uint8_t UID[UID_LEN_MAX];
    uint8_t valid_len;
}card_hf_uid;

extern card_hf_uid g_hf_uid;
extern uint8_t KEY_A_DEFAULT[KEY_LEN];
extern uint8_t KEY_B_DEFAULT[KEY_LEN];

#endif /*__HF_MODULE_H*/
