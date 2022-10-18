#include "main.h"
#include "pcd_state.h"

state_t g_hf_offline = {
   .parentState = NULL,
   .entryState = NULL,
   .transitions = (struct transition[]){
      {EVT_CONNECT, NULL, NULL, NULL, &g_hf_idle,}
   },
   .numTransitions = 1,
   .data = NULL,
   .entryAction = NULL,
   .exitAction = NULL,
};

state_t g_hf_idle = {
   .parentState = NULL,
   .entryState = NULL,
   .transitions = (struct transition[]){
      {EVT_REQA, NULL, NULL, NULL, &g_hf_ready,},
      {EVT_WUPA, NULL, NULL, NULL, &g_hf_ready,},
      {EVT_BREAK, NULL, NULL, NULL, &g_hf_offline,}
   },
   .numTransitions = 3,
   .data = NULL,
   .entryAction = NULL,
   .exitAction = NULL,
};

state_t g_hf_ready = {
   .parentState = NULL,
   .entryState = NULL,
   .transitions = (struct transition[]){
      {EVT_SELECT, NULL, NULL, NULL, &g_hf_active,},
      {EVT_ERR, NULL, NULL, NULL, &g_hf_idle,},
      {EVT_BREAK, NULL, NULL, NULL, &g_hf_offline,}
   },
   .numTransitions = 3,
   .data = NULL,
   .entryAction = NULL,
   .exitAction = NULL,
};

state_t g_hf_active = {
   .parentState = NULL,
   .entryState = NULL,
   .transitions = (struct transition[]){
      {EVT_AUTH, NULL, NULL, NULL, &g_hf_auth,},
      {EVT_ERR, NULL, NULL, NULL, &g_hf_idle,},
      {EVT_BREAK, NULL, NULL, NULL, &g_hf_offline,}
   },
   .numTransitions = 3,
   .data = NULL,
   .entryAction = NULL,
   .exitAction = NULL,
};

state_t g_hf_auth = {
   .parentState = NULL,
   .entryState = NULL,
   .transitions = (struct transition[]){
      {EVT_ERR, NULL, NULL, NULL, &g_hf_idle,},
      {EVT_BREAK, NULL, NULL, NULL, &g_hf_offline,}
   },
   .numTransitions = 2,
   .data = NULL,
   .entryAction = NULL,
   .exitAction = NULL,
};

state_t g_hf_errorState = {0};

struct stateMachine g_hf_sm = {0};

void hf_sm_init(void)
{
    stateM_init(&g_hf_sm, &g_hf_offline, &g_hf_errorState);
}
