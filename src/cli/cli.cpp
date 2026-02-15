#include "cli/cli.h"
#include "drivers/computer_serial.h"
#include "protocol/at89c51rb2_isp.h"

#include <stdio.h>

typedef struct 
{
    char ID;
    int (*command)(void);
    const char *description;
} cmd_parser_t;

int read_manufacturer_id(void);
static const cmd_parser_t command_table[] = 
{
    {'1', read_manufacturer_id, "Read manufacturer ID"},
    {'2', NULL, "Read SSB"},
    {'3', NULL, "Full Chip Erase"},
    {'4', NULL, "Program data"},
    {'5', NULL, "Display memory data"},
    {'6', NULL, "Finish flash"},
    {'7', NULL, "Read Hardware Bytes"},
};

#define CMD_COUNT sizeof(command_table) / sizeof(command_table[0])

void cli_init(void)
{
    computer_serial_print("Welcome to the AT89C51RB2 flash program\n");
    computer_serial_print("---------------------------------------\n");
    for (int i = 0; i < (int)CMD_COUNT; i++)
    {
        char line[64];
        snprintf(line, sizeof(line), "%c: %s\r\n",
                command_table[i].ID,
                command_table[i].description);
        computer_serial_print(line);
    }
}

void cli_task(void)
{
    while (1)
    {
        uint8_t c = 254;
        computer_serial_read(&c, 1);
        if (254 != c)
        {
            cli_process(c);
        }
    }
}

void cli_process(char input_id)
{
    for (int i = 0; i < (int)CMD_COUNT; i++)
    {
        if (input_id == command_table[i].ID)
        {
            if (NULL != command_table[i].command)
            {
                char buffer[48];
                snprintf(buffer, sizeof(buffer), "Processing command with ID: %c...\r\n", command_table[i].ID);
                computer_serial_print(buffer);
                command_table[i].command();
                return;
            }
        }
    }

    char buffer[34];
    snprintf(buffer, sizeof(buffer), "Unknown command: %c\r\n", input_id);
    computer_serial_print(buffer);
}

int read_manufacturer_id(void)
{
    uint8_t id[2];
    computer_serial_print("READ MANUFACTURER ID");
    int err = at89c51rb2_read_id(id);

    if (0 == err)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "ID is: %02X %02X\r\n", id[0], id[1]);
        computer_serial_print(buffer);
    }
    else 
    {
        computer_serial_print("Failed to read MCU ID");
    }
    return err;
}