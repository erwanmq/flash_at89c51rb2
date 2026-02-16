#include "protocol/at89c51rb2_isp.h"
#include "drivers/mcu_serial.h"

#define DELAY_FOR_STABILITY 5000
#define DELAY_BOOTTIME 10000
#define DELAY_RESET 20000

#define MCU_RST_PIN     4
#define MCU_PSEN_PIN    6

/* ISP functions */
/** Read functions **/
#define READ_FCT              0x05
/*** IDs ***/
#define READ_IDs              0x00
#define READ_MANUFACTURER_ID  0x00
/*** Security Bytes ***/
#define READ_SECURITY_BYTE    0x07
#define READ_SSB 0x00
#define READ_BSB 0x01

#define READ_HARDWARE_BYTE1   0x0B
#define READ_HARDWARE_BYTE2   0x00

/** Write functions **/
#define WRITE_FCT 0x03
#define ERASE     0x01
#define ERASE_BLK_0 0x00
#define ERASE_BLK_1 0x20
#define ERASE_BLK_2 0x40
#define ERASE_BLK_3 0x80
#define ERASE_BLK_4 0xC0
#define FULL_CHIP_ERASE 0x07
#define WRITE_FUSE  0x0A
#define WRITE_FUSE_BLJB 0x04
#define WRITE_BSB_SBV 0x06
#define WRITE_BSB 0x00

/** Display functions **/
#define DISPLAY_FCT 0x04
#define DISPLAY_DATA  0x00
#define BLANK_CHECK   0x01

/** Program data functions */
#define PROGRAM_DATA_FCT  0x00

static void byte_to_ascii(uint8_t byte, char *out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = hex_chars[(byte >> 4) & 0x0F];
    out[1] = hex_chars[byte & 0x0F];
}

static uint8_t hex_char_to_val(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    return 0; 
}

uint8_t ascii_to_byte(const char *in) {
    uint8_t high = hex_char_to_val(in[0]);
    uint8_t low  = hex_char_to_val(in[1]);
    return (high << 4) | low;
}

static int at89c51rb2_write_and_check(const uint8_t *buffer, uint8_t size)
{
    int err = 0;
    for (int i = 0; i < size; i++)
    {
        /* Write to MCU */
        err = mcu_serial_write(&buffer[i], 1);
        if (0 != err)
        {
            break;
        }

        uint8_t echo;
        err = mcu_serial_read(&echo, 1);
        if (0 != err)
        {
            break;
        }
        if (echo != buffer[i])
        {
            err = -1;
            break;
        }
    }

    if (0 == err)
    {
        /* Check the status */
        uint8_t status;
        mcu_serial_peek(&status);

        /* 
        *   X: checksum error
        *   P: Security error
        *   L: Security error
        *   .: OK
        */
        bool status_error   = ('X' == status || 'L' == status || 'P' == status); 
        bool status_ok      = ('.' == status);

        Serial.print("Peek values is == ");
        Serial.println(status);

        if (status_error || status_ok)
        {
            /* We empty the buffer ('X' + 'CR+LF')*/
            mcu_serial_read(NULL, 3); // discard 3 char
        }
        
        /* err will be != 0 if status_error is true */
        err = status_error;
    }
    return err;
}

static int at89c51rb2_create_frame_header_and_write(const uint8_t *buffer,
                                         uint8_t size,
                                         uint16_t address)
{
    const uint8_t record_mark = ':';
    uint8_t address_8bits[2] = { (address >> 8) & 0x0F, address & 0x0F };

    uint8_t reclen = size - 1; // We don't count the command
    uint8_t checksum = 0;
    for (int i = 0; i < (int)size; i++)
    {
        checksum += buffer[i];
    }
    checksum += reclen;
    checksum = 256 - checksum;

    const size_t MAX_FRAME_SIZE = 256;
    char bytes_with_frame[MAX_FRAME_SIZE];

    uint8_t offset = 0;
    bytes_with_frame[offset++] = record_mark;
    byte_to_ascii(reclen, &bytes_with_frame[offset]); offset += 2;
    byte_to_ascii(address_8bits[0], &bytes_with_frame[offset]); offset += 2;
    byte_to_ascii(address_8bits[1], &bytes_with_frame[offset]); offset += 2;

    for (int i = 0; i < (int)size; i++)
    {
        byte_to_ascii(buffer[i], &bytes_with_frame[offset]);
        offset += 2;
    }

    byte_to_ascii(checksum, &bytes_with_frame[offset]); offset += 2;

    return at89c51rb2_write_and_check((uint8_t*)bytes_with_frame, offset);
}

static void delay_without_cpu_stop(unsigned long ms)
{
    const unsigned long start = micros();
    while (micros() - start < ms);
}

/* Bootloader control */
int at89c51rb2_enter_bootloader(void)
{
    delay_without_cpu_stop(DELAY_BOOTTIME);

    /* Entering bootloader */
    pinMode(MCU_RST_PIN, OUTPUT);
    pinMode(MCU_PSEN_PIN, OUTPUT);
    
    digitalWrite(MCU_RST_PIN, HIGH);
    delay_without_cpu_stop(DELAY_FOR_STABILITY);
    digitalWrite(MCU_PSEN_PIN, LOW);
    delay_without_cpu_stop(DELAY_RESET);
    digitalWrite(MCU_RST_PIN, LOW);
    delay_without_cpu_stop(DELAY_FOR_STABILITY);
    digitalWrite(MCU_PSEN_PIN, HIGH);


    /* We set to input to increase impedance and let the MCU drive the pins */
    pinMode(MCU_PSEN_PIN, INPUT);

    delay_without_cpu_stop(DELAY_FOR_STABILITY);


    /* Initialize baudrate */
    const uint8_t data = 'U';

    return at89c51rb2_write_and_check(&data, 1);
}
int at89c51rb2_finish_flash(void)
{
    const uint8_t data[] = {
        WRITE_FCT,
        WRITE_FUSE,
        WRITE_FUSE_BLJB,
        1
    };

    int err = at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);

    const  uint8_t data2[] = {
        WRITE_FCT,
        WRITE_BSB_SBV,
        WRITE_BSB,
        0
    };

    err |= at89c51rb2_create_frame_header_and_write(data2, sizeof(data2), 0);

    return err;
}

/* Erase */
int at89c51rb2_erase_block(uint8_t block)
{
    const uint8_t data[] = { WRITE_FCT, ERASE, block };
    return at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);
}
int at89c51rb2_full_chip_erase(void)
{
    const uint8_t data[] = { WRITE_FCT, FULL_CHIP_ERASE };
    return at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);
}

/* Programming */
int at89c51rb2_write_program_data_chunk(const uint8_t *buffer,
                           uint8_t size,
                           uint16_t address)
{
    int err = 0;
    const int MAX_PAGE_SIZE = 128;
    const uint8_t *data = buffer;

    /* Check if the data will cross a page boundary */
    const uint16_t end_position = address + size;
    const uint16_t page_pos     = address / MAX_PAGE_SIZE + 1;
    if (address < (MAX_PAGE_SIZE * page_pos) && end_position > (MAX_PAGE_SIZE * page_pos)) // Cross a page
    {
        /* Split the data in 2 chunks */
        const uint16_t size_until_page = MAX_PAGE_SIZE * page_pos - address;

        /* Left part */
        /* size * 2 because 1 data is 2 ascii */
        err = at89c51rb2_write_program_data_chunk(data, size_until_page * 2, address); // TODO: do something with this error
        /* New address */
        address = MAX_PAGE_SIZE * page_pos;
        size    = size - size_until_page;
        data    = &buffer[size_until_page]; // Increase the pointer pos 
        
        /* Right part */
        err = at89c51rb2_write_program_data_chunk(data, size * 2, address);
    }
    else 
    {
        uint8_t data_processed[MAX_PAGE_SIZE];
        data_processed[0] = PROGRAM_DATA_FCT;
        if (MAX_PAGE_SIZE > size)
        {
            memcpy(&data_processed[1], data, size * 2);
            err = at89c51rb2_create_frame_header_and_write(data_processed, size * 2, address);
        }
        else 
        {
            err = -1;
        }
    }

    return err; 
}

int at89c51rb2_write_program_data(const uint8_t *buffer, uint8_t size)
{
    int err = 0;
    if (0 == size || NULL == buffer)
    {
        return -1;
    }

    int offset = 0;
    while (size > offset)
    {
        if (':' != buffer[offset + 0])
        {
            err = -1;
            break;
        }

        uint8_t byte_count = ascii_to_byte((const char*)&buffer[offset + 1]);
        uint8_t address_msb = ascii_to_byte((const char*)&buffer[offset + 3]);
        uint8_t address_lsb = ascii_to_byte((const char*)&buffer[offset + 5]);
        uint16_t address = (((uint16_t)address_msb << 8) | (uint16_t)address_lsb);

        const uint8_t *data = &buffer[offset + 9];

        /* byte_count * 2 because one data is 2 ascii */
        at89c51rb2_write_program_data_chunk(data, byte_count * 2, address);

        offset += 9 + byte_count * 2 + 2; // header + data + checksum
    }
}

/* Reading */
int at89c51rb2_read_data(uint8_t *buffer, uint8_t size)
{
    int err = 0;

    int index_buffer = 0;
    while (index_buffer < size) 
    {
        uint8_t b;
        err = mcu_serial_read(&b, 1);
        if (0 != err)
        {
            break;
        }

        buffer[index_buffer++] = b;
    } 
    Serial.println("EMpty buffer");
    mcu_serial_empty_buffer();
    Serial.println("buffer empty");
    return err;
}
int at89c51rb2_read_id(uint8_t buffer[2])
{
    int err = 0;
    const uint8_t data[] = { READ_FCT, READ_IDs, READ_MANUFACTURER_ID };
    err = at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);

    if (0 == err)
    {
        err = at89c51rb2_read_data(buffer, 2);
    }

    return err;
}
int at89c51rb2_read_ssb(uint8_t buffer[2])
{
    int err = 0;
    const byte data[] = { READ_FCT, READ_SECURITY_BYTE, READ_SSB };
    err = at89c51rb2_create_frame_header_and_write(data, sizeof(data), 0);

    if (0 == err)
    {
        err = at89c51rb2_read_data(buffer, 2);
    }
    return err;
}
int at89c51rb2_read_bytes(uint8_t buffer[4])
{
    int err = 0;
    const uint8_t data_hardware_byte[] = {
        READ_FCT,
        READ_HARDWARE_BYTE1,
        READ_HARDWARE_BYTE2
    };
    err |= at89c51rb2_create_frame_header_and_write(data_hardware_byte, sizeof(data_hardware_byte), 0);

    const uint8_t data_bsb_byte[] = {
        READ_FCT,
        READ_SECURITY_BYTE,
        READ_BSB
    };
    err |= at89c51rb2_create_frame_header_and_write(data_bsb_byte, sizeof(data_bsb_byte), 0);

    if (0 == err)
    {
        err = at89c51rb2_read_data(buffer, 4);
    }

    return err;
}