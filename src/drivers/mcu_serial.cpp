#include "drivers/mcu_serial.h"

#include "SoftwareSerial.h"

#define RX_PIN 11
#define TX_PIN 10
#define MCU_BAUDRATE 19200

#define TIMER_MCU_ANSWER 10000000 // 10 seconds

static SoftwareSerial mcuSerial(RX_PIN, TX_PIN);

void mcu_serial_init(void)
{
    mcuSerial.begin(MCU_BAUDRATE);
}

static int mcu_serial_wait_for_answer(void)
{
    int err = 0;
    const unsigned long start = micros();
    while ((!mcuSerial.available()) && (micros() - start < TIMER_MCU_ANSWER));

    if (!mcuSerial.available())
    {
        err = -1;
    }
    return err;
}

int mcu_serial_write(const uint8_t *bytes_to_write, uint8_t size_to_write)
{
    int err = 0;
    if (NULL != bytes_to_write)
    {
        for (int i = 0; i < size_to_write; i++)
        {
            mcuSerial.write(bytes_to_write[i]);
        }
    }
    else 
    {
        err = -1;
    }

    return err;
}

int mcu_serial_read(uint8_t *i_buffer, uint8_t size_to_read)
{
    int err = 0;
    /* Wait for a response */
    int index = 0;
    while (0 == mcu_serial_wait_for_answer())
    {
        uint8_t b = mcuSerial.read();
        if (NULL != i_buffer && index < size_to_read)
        {
            i_buffer[index++] = b;
        }

        if (!mcuSerial.available())
        {
            break;
        }
    }
    return err;
}

int mcu_serial_peek(uint8_t* i_buffer)
{
    int err = 0;
    if (NULL != i_buffer)
    {
        /* Wait for a response */
        err = mcu_serial_wait_for_answer();
        if (0 == err)
        {
            *i_buffer = (uint8_t)mcuSerial.peek();
        }
    }
    return err;
}