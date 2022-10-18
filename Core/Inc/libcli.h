#ifndef __LIBCLI_H__
#define __LIBCLI_H__

#include "main.h"

/* cli status */
#define CLI_OK 0
#define CLI_ERROR -1
#define CLI_QUIT -2
#define CLI_ERROR_ARG -3
#define CLI_AMBIGUOUS -4
#define CLI_UNRECOGNIZED -5
#define CLI_MISSING_ARGUMENT -6
#define CLI_MISSING_VALUE -7
#define CLI_INCOMPLETE_COMMAND -8

#define MAX_HISTORY 8

#define PRIVILEGE_UNPRIVILEGED 0
#define PRIVILEGE_PRIVILEGED 15
#define MODE_ANY -1
#define MODE_EXEC 0
#define MODE_CONFIG 1

/* command line parameters */
#define CLI_MAX_LINE_LENGTH 512
#define CLI_MAX_LINE_WORDS 32
#define CLI_MAX_USER_INFO 32

#define free_z(ptr)    \
    do                 \
    {                  \
        if (ptr)       \
        {              \
            free(ptr); \
            ptr = NULL;\
        }              \
    } while (0)

extern uint8_t g_cmd_parse_flag;
extern uint8_t g_cmd[CLI_MAX_LINE_LENGTH];

enum cli_states {
    STATE_LOGIN,
    STATE_PASSWORD,
    STATE_NORMAL,
    STATE_ENABLE_PASSWORD,
    STATE_ENABLE,
};

enum command_types
{
    CLI_ANY_COMMAND,
    CLI_REGULAR_COMMAND,
    CLI_FILTER_COMMAND,
};

enum optarg_flags {
    CLI_CMD_OPTIONAL_FLAG     = 1 << 0,
    CLI_CMD_OPTIONAL_ARGUMENT = 1 << 1,
    CLI_CMD_ARGUMENT          = 1 << 2,
    CLI_CMD_OPTION_MULTIPLE   = 1 << 3,
    CLI_CMD_OPTION_SEEN       = 1 << 4,
    CLI_CMD_TRANSIENT_MODE    = 1 << 5,
    CLI_CMD_DO_NOT_RECORD     = 1 << 6,
    CLI_CMD_REMAINDER_OF_LINE = 1 << 7,
    CLI_CMD_HYPHENATED_OPTION = 1 << 8,
    CLI_CMD_SPOT_CHECK        = 1 << 9,
};

struct cli_def_t;
struct cli_command_t;
typedef struct cli_def_t cli_def;
typedef struct cli_command_t cli_command;

typedef struct cli_optarg_pair_t
{
    uint8_t *name;
    uint8_t *value;
    struct cli_optarg_pair_t *next;
}cli_optarg_pair;

typedef struct user_info_t
{
    char *username;
    char *password;
    struct user_info_t *next;
}user_info;

typedef struct
{
    int comma_separated;
    uint8_t **entries;
    int num_entries;
}cli_comphelp;

typedef struct cli_optarg_t
{
    uint8_t *name;
    int flags;
    uint8_t *help;
    int mode;
    int privilege;
    uint8_t unique_len;
    int (*get_completions)(cli_def *, const uint8_t *, const uint8_t *, cli_comphelp *);
    int (*validator)(cli_def *, const uint8_t *, const uint8_t *);
    int (*transient_mode)(cli_def *, const uint8_t *, const uint8_t *);
    struct cli_optarg_t *next;
}cli_optarg;

typedef struct
{
    uint8_t *cmdline;
    uint8_t *words[CLI_MAX_LINE_WORDS];
    int num_words;
    cli_command *command;
    cli_optarg_pair *found_optargs;
    int status;
    int first_unmatched;
    int first_optarg;
    uint8_t *error_word;
}cli_msg;

struct cli_def_t
{
    int completion_callback;
    cli_command *command_table;
    uint8_t *history[MAX_HISTORY];
    int state;
    void *service;
    uint8_t *buffer;
    uint8_t buf_size;
    cli_msg *message;
    cli_optarg_pair *found_optargs;
    int transient_mode;
};

struct cli_command_t
{
    uint8_t *command;
    uint8_t *full_command_name;
    int (*callback)(cli_def *, const uint8_t *, uint8_t **, int);
    uint8_t unique_len;
    uint8_t *help;
    cli_command *previous;
    cli_command *next;
    cli_command *children;
    cli_command *parent;
    cli_optarg  *optarg_table;
    int command_type;
    int flags;
};
/*
cli_def *cli_init(void);
int cli_loop(cli_def *cli, uint8_t *buf);
*/

int cli_parse_cmd(uint8_t *command, uint16_t cmd_len);
#endif
