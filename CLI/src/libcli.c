#include "libcli.h"
#include "hf_module.h"
#include "thm3070_drv.h"
#include "MFRC522.h"

#define CTRL(c) (c - '@')

uint8_t g_cmd_parse_flag = FALSE;
uint8_t g_cmd[CLI_MAX_LINE_LENGTH] = {0};
/* perfect cli needs so much time ,it can't be finished now
static uint16_t sg_cursor = 0;
static uint16_t sg_len = 0;
static uint8_t sg_id_history = 0;

static void cli_printf(uint8_t *content, int len)
{
    static uint8_t output_buf[CLI_MAX_LINE_LENGTH] = {0};

    strncpy(output_buf, content, len);
    printf("%s", output_buf);

    memset(output_buf, 0, CLI_MAX_LINE_LENGTH);
}

static void cli_clear_line(uint8_t *cmd, int len, int cursor)
{
    // Use cmd as our buffer, and overwrite contents as needed.
    // Backspace to beginning
    memset((char *)cmd, '\b', cursor);
    cli_printf(cmd, cursor);

    // Overwrite existing cmd with spaces
    memset((char *)cmd, ' ', len);
    cli_printf(cmd, len);

    // ..and backspace again to beginning
    memset((char *)cmd, '\b', len);
    cli_printf(cmd, len);

    // Null cmd buffer
    memset((char *)cmd, 0, len);
}

int cli_parse_data(cli_def *cli, uint8_t *buf, uint16_t data_len)
{
    uint8_t c = 0;
    for (int i = 0; i < data_len; i++)
    {
        c = *(buf + i);
        
        if (c == 0) continue;
        if (c == '\n') continue;

        if (c == '\r')
        {
            printf("\r\n");
            g_cmd_parse_flag = TRUE;
            break;
        }

        if (c == CTRL('C'))
        {
            printf("\a");
            continue;
        }

        // Back word, backspace/delete
        if (c == CTRL('W') || c == CTRL('H') || c == 0x7f)
        {
            int back = 0;

            if (c == CTRL('W'))
            {
                // Word
                int nc = sg_cursor;

                if (sg_len == 0 || sg_cursor == 0) continue;

                while (nc && g_cmd[nc - 1] == ' ')
                {
                    nc--;
                    back++;
                }

                while (nc && g_cmd[nc - 1] != ' ')
                {
                    nc--;
                    back++;
                }
            }
            else
            {
                // Char
                if (sg_len == 0 || sg_cursor == 0)
                {
                     printf("\a");
                     continue;
                }

                back = 1;
            }

            if (back)
            {
                while (back--)
                {
                    if (sg_len == sg_cursor)
                    {
                        g_cmd[--sg_cursor] = 0;
                        printf("\b \b");
                    } 
                    else 
                    {
                        // Back up one space, then write current buffer followed by a space
                        printf("\b");
                        cli_printf(g_cmd + sg_cursor, sg_len - sg_cursor);
                        printf(" ");

                        // Move everything one char left
                        memmove(g_cmd + sg_cursor - 1, g_cmd + sg_cursor, sg_len - sg_cursor);

                        // Set former last char to null
                        g_cmd[sg_len - 1] = 0;

                        // And reposition cursor
                        for (int i = sg_len; i >= sg_cursor; i--)
                        {
                            printf("\b");
                        }
                    }
                    sg_cursor--;
                }
                sg_len--;
            }

            continue;
        }

        // cursor motion, Left/right arrow
        if (c == CTRL('B') || c == CTRL('F'))
        {
            if (c == CTRL('B'))
            {
                // Left
                if (sg_cursor)
                {
                    printf("\b");
                    sg_cursor--;
                }
            }
            else
            {
                // Right
                if (sg_cursor < sg_len)
                {
                    printf("%c", g_cmd[sg_cursor]);
                    sg_cursor++;
                }
            }
            continue;
        }

        // History, up/down arrow
        if (c == CTRL('P') || c == CTRL('N'))
        {
            int history_found = 0;
    
            if (c == CTRL('P'))
            {
                // Up
                sg_id_history--;
                if (sg_id_history < 0)
                {
                    for (sg_id_history = MAX_HISTORY - 1; sg_id_history >= 0; sg_id_history--)
                    {
                        if (cli->history[sg_id_history])
                        {
                            history_found = 1;
                            break;
                        }
                    }
                }
                else 
                {
                    if (cli->history[sg_id_history])
                    {
                        history_found = 1;
                    }
                }
            }
            else
            {
                // Down
                sg_id_history++;
                if (sg_id_history >= MAX_HISTORY || !cli->history[sg_id_history])
                {
                    int i = 0;
                    for (i = 0; i < MAX_HISTORY; i++)
                    {
                        if (cli->history[i])
                        {
                            sg_id_history = i;
                            history_found = 1;
                            break;
                        }
                    }
                }
                else 
                {
                    if (cli->history[sg_id_history])
                    {
                        history_found = 1;
                    }
                }
            }

            if (history_found && cli->history[sg_id_history])
            {
                // Show history item
                cli_clear_line(g_cmd, sg_len, sg_cursor);
                memset(g_cmd, 0, CLI_MAX_LINE_LENGTH);
                strncpy(g_cmd, cli->history[sg_id_history], CLI_MAX_LINE_LENGTH - 1);
                sg_len = sg_cursor = strlen(g_cmd);
                cli_printf(g_cmd, sg_len);
            }

            continue;
        }

        // Normal character typed.
        if (sg_cursor == sg_len) 
        {
            // Append to end of line if not at end-of-buffer.
            if (sg_len < CLI_MAX_LINE_LENGTH - 1)
            {
                g_cmd[sg_cursor] = c;
                sg_len++;
                sg_cursor++;
            }
            else
            {
                // End-of-buffer, ensure null terminated
                g_cmd[sg_cursor] = 0;
                printf("\a");
                continue;
            }
        }
        else
        {
            // Middle of text
            int i;
            // Move everything one character to the right
            memmove(g_cmd + sg_cursor + 1, g_cmd + sg_cursor, sg_len - sg_cursor);

            // Insert new character
            g_cmd[sg_cursor] = c;

            // IMPORTANT - if at end of buffer, set last char to NULL and don't change length, otherwise bump length by 1
            if (sg_len == CLI_MAX_LINE_LENGTH - 1) 
            {
                g_cmd[sg_len] = 0;
            }
            else
            {
                sg_len++;
            }

            // Write buffer, then backspace to where we were
            cli_printf(g_cmd + sg_cursor, sg_len - sg_cursor);
            for (i = 0; i < (sg_len - sg_cursor); i++)
            {
                printf("\b");
            }
            sg_cursor++;
        }
        
        lastchar = c;
    }
}
*/

// simple cli,just meet the requirements
#define TEST_COMMAND       "test"
#define WRITE_COMMAND      "write"
#define READ_COMMAND       "read"
#define ANTI_COL_COMMAND   "anticol"
#define CMD_MAX_WORDS_NUM 3

static void cli_trim(uint8_t *string)
{

    int beg = 0, end = strlen(string) - 1;

    while(string[beg] == ' ')
    {
        ++beg;
    }

    while(string[end] == ' ')
    {
        --end;
    }
    
    string[end + 1] = 0;
    string = string + beg;
}

int cli_parse_cmd(uint8_t *command, uint16_t cmd_len)
{
    uint8_t *words[CMD_MAX_WORDS_NUM] = {NULL};

    cli_trim(command);    
    words[0] = strtok(command, " ");
    words[1] = strtok(NULL, " ");
    words[2] = strtok(NULL, " ");

    if (!strcmp(words[0], TEST_COMMAND))
    {
        /* fix me */
        uint8_t cardstr[17] = {0};
        uint8_t cardstr_len = 0;
        uint8_t ret = 0;
/****      
        ret = thm3070_request(REQA);
        if (ret == CODE_OK)
        {
            ret = thm3070_Anticoll(cardstr, &cardstr_len);
            printf("\r\nCard FOUND! UID:0x%X 0x%X 0x%X 0x%X,BCC:0x%X\r\n", cardstr[0], cardstr[1], cardstr[2], cardstr[3], cardstr[4]);
            if (ret == CODE_OK)
            {
                ret = thm3070_selectTag(cardstr, cardstr_len);
                if (ret == CODE_OK)
                {
                    printf("select successfully\r\n");
                    ret = thm3070_mifareAuth(auth_withKeyA, 32, cardstr);
                    if (ret == CODE_OK)
                    {
                        printf("auth successfully\r\n");
                    }
                    else
                    {
                        printf("auth failed\r\n");
                    }
                }
            }
        }
        else
        {
            printf("\r\nfind no card\r\n");
        }
****/
        uint8_t status = MFRC522_Request(PICC_REQIDL, cardstr);        
        if (status == MI_OK)
        {
            status = MFRC522_Anticoll(cardstr);
            printf("\r\nCard FOUND! UID:0x%X 0x%X 0x%X 0x%X,BCC:0x%X\r\n", cardstr[0], cardstr[1], cardstr[2], cardstr[3], cardstr[4]);
            if (status == MI_OK)
            {
                status = MFRC522_SelectTag(cardstr);
                if (status > 0)
                {
                    /**
                    uint8_t test_keyA[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
                    printf("select successfully\r\n");
                    status = MFRC522_Auth(PICC_AUTHENT1A, 4, test_keyA, cardstr);
                    if (status == MI_OK)
                    {
                        uint8_t block = 4;
                        uint8_t test_tx_data[16] = {0x11, 0x22, 0x33, 0x44};
                        uint8_t test_rx_data[16] = {0};
                        
                        printf("card authed\r\n");
                        if (words[1] != NULL)
                        {
                            block = strtol(words[1], NULL, 16);
                        }
                        MFRC522_Write(block, test_tx_data);
                        status = MFRC522_Read(block, test_rx_data);

                        if (!strncmp(test_tx_data, test_rx_data, 16))
                        {
                            printf("card write succeed\r\n");
                        }
                        else
                        {
                            printf("[ERR]:write failed!\r\n");
                        }
                    }
                    else
                    {
                        printf("[ERR]:auth failed!\r\n");
                    }
                    **/
                }
                else
                {
                    printf("[ERR]:select failed!\r\n");
                }
                    
            }
            else
            {
                printf("\r\n[ERR]something wrong with anti-collision\r\n");
            }
        }
        else
        {
            printf("\r\n[WARINING]find no card\r\n");
        }
        //printf("\r\n write register \r\n");
    }
    else if (!strcmp(words[0], ANTI_COL_COMMAND))
    {   
        // MALLOC_CHECK(g_card_tbl, sizeof(struct binary_tree), 1);
        // MALLOC_CHECK(g_card_tbl->value, sizeof(card_hf_uid), 1);
        if (MI_OK == MFRC522_Request(PICC_REQIDL, NULL))
        {
            uint8_t status = tree_creater(&g_card_tbl, sizeof(card_hf_uid));
            
            if (MI_TAGFOUND & anti_collision_loop(g_card_tbl, 1))
            {
                printf("\r\nanti_collision_loop succeed!");
                HF_TRAVERSE_CALLBACK(level, g_card_tbl, cb_hf_module_node_print);
            }
            tree_destructor(g_card_tbl);
        }
        else
        {
            printf("\r\n[WARINING]find no card\r\n");
        }
    }
    else if (!strcmp(words[0], READ_COMMAND))
    {
        //rc522_pkt_set(&g_rc522_spi, ControlReg, SPI_READ_CMD, NULL, 0, 1);
        //rc522_spi_transmit(&g_rc522_spi);
        uint8_t reg = strtol(words[1], NULL, 16);
        uint8_t ret = Read_MFRC522(reg);
        //uint8_t ret = thm3070_read_reg(reg);
        printf("\r\n$MCU > reg:0x%X,data:0x%X\r\n", reg, ret);
    }
    else if (!strcmp(words[0], WRITE_COMMAND))
    {
        uint8_t reg  = strtol(words[1], NULL, 16);
        uint8_t data = strtol(words[2], NULL, 16);
        Write_MFRC522(reg, data);
        printf("\r\n$MCU > write reg:0x%X successfully\r\n", reg);
    }
    
    printf("\r\n");
    memset(command, 0, cmd_len);
    return CLI_OK;
}
