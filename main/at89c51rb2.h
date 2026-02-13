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
  Error err = OK;
  int index_buffer = 0;

  
  // if (index_buffer < size_buf)
  // {
  //   buffer[index_buffer++] = b;
  // }
  Serial.println();
  const unsigned long start = micros();
  while((!mcuSerial.available()) && (micros() - start < TIMER_LOOP));
  byte b;
  do {
    unsigned long start = micros();
    while((!mcuSerial.available()) && (micros() - start < TIMER_LOOP));
    if (!mcuSerial.available())
    {
      break;
    }
    b = mcuSerial.read();
    /* Write to the console for debugging purpose. */
    if (b < 0x20 || b > 0x7E) {
        Serial.print(" ");
        Serial.print("0x");
        if (b < 0x10) Serial.print('0');
        Serial.print(b, HEX);
        Serial.print(" ");
    } else {
        Serial.print((char)b);
    }
  } while (micros() - start < TIMER_MCU_ANSWER);

  Serial.println("Read finished");
  mcuSerial.readStringUntil('\n'); // Empty the buffer

  
  /* Based on the value, we can know the MCU state. */
  // switch (b)
  // {
  //   case 'X':
  //     err = CHECKSUM_ERR;
  //     break;
    
  //   case 'P':
  //     err = SECURITY_ERR;
  //     break;
    
  //   case '.':    
  //   case '\r':
  //     err = OK;
  //     break;
    
  //   default: {}
  // }
  Serial.println();

  return err;
}

Error write(const byte *bytes, uint8_t size)
{
  Error err = OK;
  for (int i = 0; i < size; i++)
  {
    /* Write to MCU */
    mcuSerial.write(bytes[i]);

    /* Wait its echo */
    unsigned long start = micros();
    while((!mcuSerial.available()) && (micros() - start < TIMER_LOOP));

    if (mcuSerial.available())
    {
      byte echo = mcuSerial.read();
      if (echo != bytes[i])
      {
        Serial.print("MCU didn't echo back the correct value: ");
        Serial.print((char)echo);
        Serial.print(" original: ");
        Serial.println((char)bytes[i]);
        err = ERR;
        break;
      } 
      else 
      {
        Serial.print((char)echo);
        Serial.print(" ");
      }
    }
    else 
    {
      err = TIMEOUT_ERR;
      break;
    }
  }
  return err;
}

Error write_to_mcu(const byte *bytes, uint8_t size)
{
  Error err = OK;
  
  err = write(bytes, size);

  /* Check the end of transmission */
  byte cr_lf[2];
  for (int i = 0; i < sizeof(cr_lf); i++)
  {
    unsigned long start = micros();
    while((!mcuSerial.available()) && (micros() - start < TIMER_MCU_ANSWER));
    if (mcuSerial.available())
    {
      byte b = mcuSerial.read();
      Serial.print((int)b);
      if ('\r' != b && '\n' != b)
      {
        Serial.print((char)b);
        Serial.println();
        i--;
      }
      else
      {
        cr_lf[i] = b;
      }
      
    }
    else 
    {
      Serial.println("Error, no CRLF ending");
      err = ERR; 
      break;
    }
  }

  if ('\r' != cr_lf[0] || '\n' != cr_lf[1])
  {
    Serial.println("Error, there is no CRLF");
    err = ERR;
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
  Error err = OK;
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

  err = write_to_mcu(bytes_with_frame, offset);

  if (OK == err)
  {
    err = read_mcu_serial();
  }
  else
  {
    Serial.println("Fail to write in create_frame_header_and_write");
  }
  return err;
}

Error initialize_baudrate()
{
  byte data = 'U';
  Error err = write(&data, 1); // Don't need a frame for the auto baudrate

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
  const int max_page_size = 128;
  byte data[max_page_size + 1]; // + 1 for the command
  data[0] = { PROGRAM_DATA_FCT }; 
  Error err = OK;

  int data_index = 0;
  int page_sent = 0;
  for (int i = 0 ; i < size; i++)
  {
    data[data_index + 1] = bytes[i];
    data_index++;
    if (data_index >= max_page_size)
    {
      err = create_frame_header_and_write(data, sizeof(data_index), page_sent * max_page_size);
      data_index = 0;
      page_sent++;
      if (OK != err)
      {
        return err;
      }
    }
  }
  return create_frame_header_and_write(data, data_index + 1, page_sent * max_page_size);
}

Error display_memory(byte start_address[2], byte end_address[2])
{
  byte data[] = { 
    DISPLAY_FCT, 
    start_address[0], start_address[1], 
    end_address[0], end_address[1], 
    DISPLAY_DATA
  };

  Serial.print("Address requested = ");
  Serial.print(start_address[0]);
  Serial.print(" - ");
  Serial.println(start_address[1]);


  return create_frame_header_and_write(data, sizeof(data), 0);
}

Error read_bytes()
{
  Error err = OK;
  byte data_hardware_byte[] = {
    READ_FCT,
    READ_HARDWARE_BYTE1,
    READ_HARDWARE_BYTE2
  };
  err |= create_frame_header_and_write(data_hardware_byte, sizeof(data_hardware_byte), 0);

  byte data_bsb_byte[] = {
    READ_FCT,
    READ_SECURITY_BYTE,
    READ_BSB
  };
  err |= create_frame_header_and_write(data_bsb_byte, sizeof(data_bsb_byte), 0);

  return err;
}

/* 1 = User's application
   0 = Bootloader
*/
Error finish_flash()
{
  byte data[] = {
    WRITE_FCT,
    WRITE_FUSE,
    WRITE_FUSE_BLJB,
    1
  };

  Error err = create_frame_header_and_write(data, sizeof(data), 0);

  byte data2[] = {
    WRITE_FCT,
    WRITE_BSB_SBV,
    WRITE_BSB,
    0
  };

  err |= create_frame_header_and_write(data2, sizeof(data2), 0);

  return err;
}
