#include "SoftwareSerial.h"

#ifdef __AVR_ATmega2560__
  const byte CLOCKOUT = 11;  // Mega 2560
#else
  const byte CLOCKOUT = 9;   // Uno, Duemilanove, etc.
#endif

const byte RST_PIN  = 4;
const byte PSEN_PIN = 6;
const byte TX_PIN   = 10;
const byte RX_PIN   = 11;

SoftwareSerial mcuSerial(RX_PIN, TX_PIN);

void enterBootloader()
{
  pinMode(RST_PIN, OUTPUT);
  pinMode(PSEN_PIN, OUTPUT);

  digitalWrite(RST_PIN, HIGH);
  unsigned long start = micros();
  while (micros() - start < 5000);
  digitalWrite(PSEN_PIN, LOW);
  start = micros();
  while (micros() - start < 20000);
  digitalWrite(RST_PIN, LOW);
  start = micros();
  while (micros() - start < 5000);
  digitalWrite(PSEN_PIN, HIGH);


  pinMode(PSEN_PIN, INPUT);
  pinMode(RST_PIN, INPUT);

  start = micros();
  while (micros() - start < 10000);
}

void setup ()
{
  mcuSerial.begin(19200);
  Serial.begin(19200);

  unsigned long start = micros();
  while (micros() - start < 100000);
  enterBootloader();

  mcuSerial.write("U");
  while(!mcuSerial.available());
  Serial.write(mcuSerial.read());


  byte data[] = {0x01, 0x00, 0x10, 0x00, 0x55, 0x9A};
  mcuSerial.write(data, sizeof(data));

  while(mcuSerial.available())
  {
    Serial.write('\n');
    byte incoming = mcuSerial.read();
    Serial.println(incoming, DEC);
  }
}  // end of setup

void loop ()
{
  while (Serial.available())
    mcuSerial.write(Serial.read());
  while (mcuSerial.available())
    Serial.write(mcuSerial.read());
}  // end of loop
