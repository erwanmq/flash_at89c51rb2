#include "drivers/computer_serial.h"

#define COMPUTER_BAUDRATE 19200
#define TIMER_COMPUTER_ANSWER 10000000 // 10 seconds

void computer_serial_init()
{
    Serial.begin(COMPUTER_BAUDRATE);
}

static int computer_serial_wait_for_answer()
{
    int err = 0;
    const unsigned long start = micros();
    while ((!Serial.available()) && (micros() - start < TIMER_COMPUTER_ANSWER));

    if (!Serial.available())
    {
        err = -1;
    }
    return err;
}

int computer_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write)
{
    int err = 0;
    if (NULL != bytes_to_write)
    {
        for (int i = 0; i < size_to_write; i++)
        {
            Serial.write(bytes_to_write[i]);
        }
    }
    else 
    {
        err = -1;
    }
    return err;
}

void computer_serial_print(const char* buffer)
{
    if (NULL != buffer)
    {
        Serial.print(buffer);
    }
}

int computer_serial_read(uint8_t *i_buffer, uint8_t size_to_read)
{
    int err = 0;
    int index = 0;
    while (0 == computer_serial_wait_for_answer())
    {
        uint8_t b = Serial.read();
        if (NULL != i_buffer && index < size_to_read)
        {
            i_buffer[index++] = b;
        }

        if (!Serial.available())
        {
            break;
        }
    }
    return err;
}

int computer_serial_peek(uint8_t *i_buffer)
{
    int err = 0;
    if (NULL != i_buffer)
    {
        /* Wait for a response */
        err = computer_serial_wait_for_answer();
        if (0 != err)
        {
            i_buffer = Serial.peek();
        }
    }
    return err;
}