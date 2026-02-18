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

int cli_read_manufacturer_id(void);
int cli_read_ssb(void);
int cli_full_chip_erase(void);
int cli_program_data(void);
int cli_display_memory(void);
int cli_finish_flash(void);
int cli_read_hardware_bytes(void);
static const cmd_parser_t command_table[] = 
{
    {'1', cli_read_manufacturer_id, "Read manufacturer ID"},
    {'2', cli_read_ssb, "Read SSB"},
    {'3', cli_full_chip_erase, "Full Chip Erase"},
    {'4', cli_program_data, "Program data"},
    {'5', cli_display_memory, "Display memory data"},
    {'6', cli_finish_flash, "Finish flash"},
    {'7', cli_read_hardware_bytes, "Read Hardware Bytes"},
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
        computer_serial_empty_buffer();
        if (254 != c)
        {
            cli_process(c);
            computer_serial_empty_buffer();
            cli_init();
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

int cli_read_manufacturer_id(void)
{
    uint8_t id[2];
    int err = at89c51rb2_read_id(id);

    if (0 == err)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "ID is: %c %c\r\n", id[0], id[1]);
        computer_serial_print(buffer);
    }
    else 
    {
        computer_serial_print("Failed to read MCU ID\n");
    }
    return err;
}

int cli_read_ssb(void)
{
    uint8_t ssb[2];
    int err = at89c51rb2_read_ssb(ssb);

    if (0 == err)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "SSB value is: %c %c\r\n", ssb[0], ssb[1]);
        computer_serial_print(buffer);
    }
    else 
    {
        computer_serial_print("Failed to read MCU SSB\n");
    }
    return err;
}

int cli_full_chip_erase(void)
{
    int err = at89c51rb2_full_chip_erase();
    if (0 == err)
    {
        computer_serial_print("Chip full erased\n");
    }
    else 
    {
        computer_serial_print("Error during full erase\n");
    }
    return err;
}

int cli_program_data(void)
{
    computer_serial_print("Enter your hex intel data in one row with the semicolon: ");

    uint8_t program_data[128] = { 0 };
    int size = computer_serial_read(program_data, sizeof(program_data));
    computer_serial_empty_buffer();

    int err = at89c51rb2_write_program_data(program_data, size);
    
    return err;
}

int cli_display_memory(void)
{
    char start_address[4];
    char end_address[4];
    computer_serial_print("Enter the start address MSB: \n");
    computer_serial_read((uint8_t*)start_address, 2);
    computer_serial_empty_buffer();
    
    computer_serial_print("Enter the start address LSB: \n");
    computer_serial_read((uint8_t*)&start_address[2], 2);
    computer_serial_empty_buffer();

    computer_serial_print("Enter the end address MSB: \n");
    computer_serial_read((uint8_t*)end_address, 2);
    computer_serial_empty_buffer();

    computer_serial_print("Enter the end address LSB: \n");
    computer_serial_read((uint8_t*)&end_address[2], 2);
    computer_serial_empty_buffer();

    uint8_t buffer[128];
    int err = at89c51rb2_display_memory(start_address, end_address, buffer, sizeof(buffer));
    if (0 == err)
    {
        computer_serial_print("Memory is:\n");
        computer_serial_print((const char*)buffer);
    }
    else 
    {
        computer_serial_print("Failed to fetch the memory\n");
    }
    return err;
}
int cli_finish_flash(void)
{
    int err = at89c51rb2_finish_flash();

    if (0 == err)
    {
        computer_serial_print("Flash finished, you can disconnect the MCU\n");
    }
    else 
    {
        computer_serial_print("Finish flash error\n");
    }
    return err;
}

int cli_read_hardware_bytes(void)
{
    int err = 0;
    computer_serial_print("Not implemented\n");
    return err;
}