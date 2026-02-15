#ifndef MCU_SERIAL_H__
#define MCU_SERIAL_H__

#include "Arduino.h"

void mcu_serial_init(void);

/* Used to write to the Serial */
int mcu_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write);

/* Used to read from the Serial */
int mcu_serial_read(uint8_t *i_buffer, uint8_t size_to_read);

int mcu_serial_peek(uint8_t* i_buffer);

#endif /* MCU_SERIAL_H__ */