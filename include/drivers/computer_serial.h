#ifndef COMPUTER_SERIAL_H__
#define COMPUTER_SERIAL_H__

#include "Arduino.h"


void computer_serial_init(void);

/* Used to write to the Serial */
int computer_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write);
void computer_serial_print(const char* buffer);

/* Used to read from the Serial */
int computer_serial_read(uint8_t *i_buffer, uint8_t size_to_read);

#endif /* COMPUTER_SERIAL_H__ */