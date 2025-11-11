#include "SoftwareSerial.h"

/* Delay in ms */
#define DELAY_BOOTTIME      10000 
#define DELAY_RESET         20000
#define DELAY_FOR_STABILITY 5000
#define TIMER_LOOP          50000
#define TIMER_MCU_ANSWER    10000000 // 10 seconds

#define BAUDRATE  19200

/* ISP functions */
/** Read functions **/
#define READ_FCT              0x05
/*** IDs ***/
#define READ_IDs              0x00
#define READ_MANUFACTURER_ID  0x00
/*** Security Bytes ***/
#define READ_SECURITY_BYTE    0x07
#define READ_SSB 0x00

/** Write functions **/
#define WRITE_FCT 0x03
#define ERASE     0x01
#define ERASE_BLK_0 0x00
#define ERASE_BLK_1 0x20
#define ERASE_BLK_2 0x40
#define ERASE_BLK_3 0x80
#define ERASE_BLK_4 0xC0
#define FULL_CHIP_ERASE 0x07

/** Display functions **/
#define DISPLAY_FCT 0x04
#define DISPLAY_DATA  0x00
#define BLANK_CHECK   0x01

/** Program data functions */
#define PROGRAM_DATA_FCT  0x00

enum Error
{
  OK,
  TIMEOUT_ERR,
  CHECKSUM_ERR,
  SECURITY_ERR,
  ERR,
};

void stopProgram() {
    noInterrupts(); 
    while (true) {
        
    }
}

#define CHECK_ERROR(err, msg) \
do { \
  if (OK != err) \
  { \
    char buf[64]; \
    sprintf(buf, "%s Error nb: %d", msg, err); \
    Serial.println(buf); \
    Serial.flush(); \
    stopProgram(); \
  } \
} while (0)

#define CHECK_ERROR_WITH_RETURN(err, msg) \
do { \
  if (OK != err) \
  { \
    char buf[64]; \
    sprintf(buf, "%s Error nb: %d", msg, err); \
    Serial.println(buf); \
    Serial.flush(); \
    return err; \
  } \
} while (0)

const byte RST_PIN  = 4;
const byte PSEN_PIN = 6;
const byte TX_PIN   = 10;
const byte RX_PIN   = 11;

SoftwareSerial mcuSerial(RX_PIN, TX_PIN);

void delay_without_stop(unsigned long ms)
{
  unsigned long start = micros();
  while (micros() - start < ms);
}

void enterBootloader()
{
  pinMode(RST_PIN, OUTPUT);
  pinMode(PSEN_PIN, OUTPUT);

  digitalWrite(RST_PIN, HIGH);
  delay_without_stop(DELAY_FOR_STABILITY);
  digitalWrite(PSEN_PIN, LOW);
  delay_without_stop(DELAY_RESET);
  digitalWrite(RST_PIN, LOW);
  delay_without_stop(DELAY_FOR_STABILITY);
  digitalWrite(PSEN_PIN, HIGH);

  /* We set to input to increase impedance and let the MCU drive the pins */
  pinMode(PSEN_PIN, INPUT);
  //pinMode(RST_PIN, INPUT);

  delay_without_stop(DELAY_FOR_STABILITY);
}

Error read_mcu_serial()
{
  Error err = ERR;
  if (!mcuSerial.available())
  {
    return err; // MCU should echo back
  }

  const unsigned long start = micros();
  byte b = mcuSerial.read();
  while (('X' != b && 'P' != b && '.' != b) && (micros() - start < TIMER_MCU_ANSWER)) {
    /* Write to the console for debugging purpose. */
    if (b < 0x20 || b > 0x7E) {
        Serial.print("0x");
        if (b < 0x10) Serial.print('0');
        Serial.print(b, HEX);
        Serial.print(" ");
    } else {
        Serial.print((char)b);
    }
    b = mcuSerial.read();
  }
  /* Based on the value, we can know the MCU state. */
  switch (b)
  {
    case 'X':
      err = CHECKSUM_ERR;
      break;
    
    case 'P':
      err = SECURITY_ERR;
      break;
    
    case '.':
      err = OK;
      break;
    
    default: {}
  }
  Serial.println();

  return err;
}

Error write_to_mcu(const byte *bytes, uint8_t size)
{
  Error err = OK;
  mcuSerial.write(bytes, size);
  unsigned long start = micros();
  while((!mcuSerial.available()) && (micros() - start < TIMER_LOOP));

  if (!mcuSerial.available())
  {
    err = TIMEOUT_ERR;
  }
  return err;
}

void byte_to_hex(uint8_t byte, char *out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = hex_chars[(byte >> 4) & 0x0F];
    out[1] = hex_chars[byte & 0x0F];
}

Error create_frame_header_and_write(const byte *bytes, uint8_t size, uint16_t load_offset)
{
  const byte record_mark = ':';
  byte load_offset_8bit[2] = { (load_offset >> 8) & 0x0F, load_offset & 0x0F };

  byte reclen = size - 1; // + 1 for checksum, -1 for the main command
  byte checksum = 0;
  for (uint8_t i = 0; i < size; i++)
  {
    checksum += bytes[i];
  }
  checksum += reclen;
  checksum = 256 - checksum; // TODO: remove the 256 magic number

  const size_t MAX_FRAME_SIZE = 256;
  byte bytes_with_frame[MAX_FRAME_SIZE];

  size_t offset = 0;
  bytes_with_frame[offset++] = record_mark;
  byte_to_hex(reclen, &bytes_with_frame[offset]); offset += 2;
  byte_to_hex(load_offset_8bit[0], &bytes_with_frame[offset]); offset += 2;
  byte_to_hex(load_offset_8bit[1], &bytes_with_frame[offset]); offset += 2;

  for (uint8_t i = 0; i < size; i++)
  {
    byte_to_hex(bytes[i], &bytes_with_frame[offset]);
    offset += 2;
  }

  byte_to_hex(checksum, &bytes_with_frame[offset]); offset += 2;

  Error err = write_to_mcu(bytes_with_frame, offset);
  CHECK_ERROR_WITH_RETURN(err, "write_to_mcu in create_frame_header_and_write");

  return read_mcu_serial();
}

Error initialize_baudrate()
{
  byte data = 'U';
  Error err = write_to_mcu(&data, 1); // Don't need a frame for the auto baudrate
  CHECK_ERROR_WITH_RETURN(err, "write_to_mcu in initialize_baudrate");

  /* Empty the UART buffer */
  while(mcuSerial.available()) { mcuSerial.read(); }
  return err;
}

Error read_mcu_id()
{
  const byte data[] = { READ_FCT, READ_IDs, READ_MANUFACTURER_ID };
  return create_frame_header_and_write(data, sizeof(data), 0);
}

Error erase_block(int block)
{
  byte data[] = { WRITE_FCT, ERASE, block };
  return create_frame_header_and_write(data, sizeof(data), 0);
}

Error full_chip_erase()
{
  const byte data[] = { WRITE_FCT, FULL_CHIP_ERASE };
  return create_frame_header_and_write(data, sizeof(data), 0);
}

Error read_SSB()
{
  const byte data[] = { READ_FCT, READ_SECURITY_BYTE, READ_SSB };
  return create_frame_header_and_write(data, sizeof(data), 0);
}

Error write_program_data(byte* bytes, int size)
{
  byte data[size + 1] = { PROGRAM_DATA_FCT };
  for (int i = 0 ; i < size; i++)
  {
    data[i + 1] = bytes[i];
  }
  return create_frame_header_and_write(data, sizeof(data), 0);
}

Error display_memory(byte start_address[2], byte end_address[2])
{
  byte data[] = { 
    DISPLAY_FCT, 
    start_address[0], start_address[1], 
    end_address[0], end_address[1], 
    DISPLAY_DATA
  };

  return create_frame_header_and_write(data, sizeof(data), 0);
}
