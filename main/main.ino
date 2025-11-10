#include "SoftwareSerial.h"

/* Delay in ms */
#define DELAY_BOOTTIME      10000 
#define DELAY_RESET         20000
#define DELAY_FOR_STABILITY 5000
#define TIMER_LOOP          50000

#define BAUDRATE  19200

/* ISP functions */
/** Read functions **/
#define READ_FCT              0x05
#define READ_IDs              0x00
#define READ_MANUFACTURER_ID  0x00

/** Write functions **/
#define WRITE_FCT 0x03
#define ERASE     0x01
#define ERASE_BLK_0 0x00
#define ERASE_BLK_1 0x20
#define ERASE_BLK_2 0x40
#define ERASE_BLK_3 0x80
#define ERASE_BLK_4 0xC0

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

bool write_to_mcu(const byte *bytes, uint8_t size)
{
  mcuSerial.write(bytes, size);
  unsigned long start = micros();
  while((!mcuSerial.available()) && (micros() - start < TIMER_LOOP));

  if (!mcuSerial.available())
  {
    return false;
  }
  return true;
}

void byte_to_hex(uint8_t byte, char *out) {
    const char hex_chars[] = "0123456789ABCDEF";
    out[0] = hex_chars[(byte >> 4) & 0x0F];
    out[1] = hex_chars[byte & 0x0F];
}

bool create_frame_header_and_write(const byte *bytes, uint8_t size)
{
  const byte record_mark = ':';
  byte load_offset[2] = { 0x00, 0x00 };

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
  byte_to_hex(load_offset[0], &bytes_with_frame[offset]); offset += 2;
  byte_to_hex(load_offset[1], &bytes_with_frame[offset]); offset += 2;

  for (uint8_t i = 0; i < size; i++)
  {
    byte_to_hex(bytes[i], &bytes_with_frame[offset]);
    offset += 2;
  }

  byte_to_hex(checksum, &bytes_with_frame[offset]); offset += 2;

  return write_to_mcu(bytes_with_frame, offset);
}

bool initialize_baudrate()
{
  byte data = 'U';
  return write_to_mcu(&data, 1); // Don't need a frame for the auto baudrate
}

bool read_mcu_id()
{
  const byte data[] = { READ_FCT, READ_IDs, READ_MANUFACTURER_ID };
  return create_frame_header_and_write(data, sizeof(data));
}

bool erase_blocks(int block)
{
  byte data[] = { WRITE_FCT, ERASE, block };
  return create_frame_header_and_write(data, sizeof(data));
}

void read_mcu_serial()
{
  while (mcuSerial.available()) {
    byte b = mcuSerial.read();
    if (b < 0x20 || b > 0x7E) {
        Serial.print("0x");
        if (b < 0x10) Serial.print('0');
        Serial.print(b, HEX);
        Serial.print(" ");
    } else {
        Serial.print((char)b);
    }
  }
  Serial.println();
}

void setup ()
{
  mcuSerial.begin(BAUDRATE);
  Serial.begin(BAUDRATE);

  delay_without_stop(DELAY_BOOTTIME);
  enterBootloader();

  /* MCU entered in the bootloader stage. 
  We have to send the character "U" and wait for its response. */
  if (!initialize_baudrate())
  {
    Serial.print("Timeout\n");
    return; // We stop the program
  }
  read_mcu_serial();

  /* Check if the MCU is responding to a command. */
  if (!read_mcu_id())
  {
    Serial.print("Timeout\n");
    return; // We stop the program
  }
  read_mcu_serial();

  erase_blocks(ERASE_BLK_0);
}

void loop ()
{

}  
