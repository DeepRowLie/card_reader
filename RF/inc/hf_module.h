#ifndef __HF_MODULE_H
#define __HF_MODULE_H

#include <stdio.h>
#include "main.h"
#include "binary_tree.h"

#define UID_LEN_MAX 15
#define BLOCK_ID    4
#define BLOCK_LEN   0x10
#define KEY_LEN     6

#define NODE_VALIDATE(node) (((node) != NULL) && ((node)->value !=NULL))

#define NODE_PRINT(node) \
        do\
        {\
            card_hf_uid *p_uid = (node)->value;\
            if (p_uid->valid_len != 0)\
            {\
                printf("\r\n############################");\
                printf("\r\nvalid_len(DEC):%d\r\nUID(HEX):", p_uid->valid_len);\
                for (uint8_t i = 0; i < ((p_uid->valid_len >> 3) + ((p_uid->valid_len % 8) > 0)); ++i)\
                {\
                    printf("0x%x ", (p_uid->UID)[i]);\
                }\
            }\
        }while(0)

typedef struct
{
    uint8_t UID[UID_LEN_MAX];
    uint8_t valid_len;
}card_hf_uid;

typedef struct binary_tree *header;

extern header g_card_tbl;
extern card_hf_uid g_hf_uid;
extern uint8_t KEY_A_DEFAULT[KEY_LEN];
extern uint8_t KEY_B_DEFAULT[KEY_LEN];

uint8_t anti_collision_loop(node root, uint8_t cascade);
void cb_hf_module_level_traverse(node root);

#endif /*__HF_MODULE_H*/
