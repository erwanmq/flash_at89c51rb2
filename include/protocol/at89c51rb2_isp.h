#ifndef PROTOCOL_AT89C51RB2_H__
#define PROTOCOL_AT89C51RB2_H__

#include "Arduino.h"

/* Bootloader control */
int at89c51rb2_enter_bootloader(void);
int at89c51rb2_finish_flash(void);

/* Erase */
int at89c51rb2_erase_block(uint8_t block);
int at89c51rb2_full_chip_erase(void);

/* Programming */
int at89c51rb2_write_program_data(const uint8_t *buffer,
                           uint8_t size,
                           uint16_t address);

/* Reading */
int at89c51rb2_read_serial(uint8_t *buffer, uint8_t size);
int at89c51rb2_read_id(uint8_t buffer[2]);
int at89c51rb2_read_ssb(uint8_t buffer[2]);
int at89c51rb2_read_bytes(uint8_t *buffer);

#endif /* PROTOCOL_AT89C51RB2_H__ */