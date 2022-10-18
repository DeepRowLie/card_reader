#ifndef __PCD_STATE_H
#define __PCD_STATE_H

#include "stateMachine.h"

typedef enum {
    EVT_ERR = 0,
    EVT_CONNECT,
    EVT_BREAK,
    EVT_REQA,
    EVT_WUPA,
    EVT_SELECT,
    EVT_AUTH,
}hf_event;

typedef struct state state_t;

extern struct stateMachine g_hf_sm;
extern state_t g_hf_offline, g_hf_idle, g_hf_ready, g_hf_active, g_hf_auth, g_hf_errorState;

void hf_sm_init(void);

#endif  /*__PCD_STATE_H*/
