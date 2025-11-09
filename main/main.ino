#include "SoftwareSerial.h"

/* Delay in ms */
#define DELAY_BOOTTIME      10000 
#define DELAY_RESET         20000
#define DELAY_FOR_STABILITY 5000
#define TIMER_LOOP          5000

/* ISP functions */
/** Read functions **/
#define READ_FCT              0x05
#define READ_IDs              0x00
#define READ_MANUFACTURER_ID  0x00

/** Write functions **/
#define WRITE_FCT 0x03
#define ERASE     0x01;
#define ERASE_BLK_0 0x00
#define ERASE_BLK_1 0x20
#define ERASE_BLK_2 0x40
#define ERASE_BLK_3 0x80
#define ERASE_BLK_4 0xC0

// TODO: Change the unsigned char to uint8_t
typedef struct 
{
  const unsigned char record_mark = ":";
  unsigned char reclen;
  unsigned char load_offset[2];
  unsigned char record_type;
  unsigned char *data;
  unsigned char checksum;
} frame_header_t;


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
  pinMode(RST_PIN, INPUT);

  delay_without_stop(DELAY_FOR_STABILITY);
}

bool write_to_mcu(const byte *bytes, unsigned char size, SoftwareSerial &mcuSerial)
{
  unsigned char reclen = size + 1 - 1; // + 1 for checksum, -1 for the main command
  unsigned char checksum = 0;
  for (unsigned char i = 0; i < size; i++)
  {
    checksum += bytes[i];
  }
  checksum += reclen;

  frame_header_t frame_header = {
    .reclen = reclen, 
    .load_offset = 0,
    .record_type = bytes[0],
    .data = (bytes + 1),
    .checksum = 256 - checksum; // TODO: remove the 256 magic number
  };

  // TODO: Implement the frame header

  mcuSerial.write(bytes, size);
  unsigned long start = micros();
  while((!mcuSerial.available()) && (micros() - start < TIMER_LOOP));

  if (!mcuSerial.available())
  {
    return false;
  }
  return true;
}

bool initialize_baudrate(SoftwareSerial &mcuSerial)
{
  const byte data[] = { "U" };
  return write_to_mcu(data, sizeof(data), mcuSerial);
}

bool read_mcu_id(SoftwareSerial &mcuSerial)
{
  const byte data[] = { READ_FCT, READ_IDs, READ_MANUFACTURER_ID };
  return write_to_mcu(data, sizeof(data), mcuSerial);
}

void setup ()
{
  mcuSerial.begin(19200);
  Serial.begin(19200);

  delay_without_stop(DELAY_BOOTTIME);
  enterBootloader();

  /* MCU entered in the bootloader stage. 
  We have to send the character "U" and wait for its response. */
  if (!initialize_baudrate(mcuSerial))
  {
    Serial.write("Timeout");
    return; // We stop the program
  }
  Serial.write(mcuSerial.read());

  /* Check if the MCU is responding to a command. */
  if (!read_mcu_id(mcuSerial))
  {
    Serial.write("Timeout");
    return; // We stop the program
  }
  while(mcuSerial.available())
  {
    Serial.write(mcuSerial.read());
  }




  // byte data[] = {0x01, 0x00, 0x10, 0x00, 0x55, 0x9A};
  // mcuSerial.write(data, sizeof(data));

  // while(mcuSerial.available())
  // {
  //   Serial.write('\n');
  //   byte incoming = mcuSerial.read();
  //   Serial.println(incoming, DEC);
  // }
}

void loop ()
{

}  
