#include "at89c51rb2.h"

void remove_unused_serial_char()
{
  while(Serial.available()) { Serial.read(); } // Read remaining unused characters.
}

byte read_answer()
{
  byte b = Serial.read();
  Serial.readStringUntil('\n'); // Remove '\n' carriage
  return b;
}

Error erase_blocks()
{
  Error err = OK;
  remove_unused_serial_char();
  Serial.println("Block 0, 1, 2, 3, 4: ");
  while(!Serial.available()); // Wait for the answer
  int second_choice = read_answer();
  switch (second_choice)
  {
    case '0':
      err = erase_block(ERASE_BLK_0);
      break;
    case '1':
      err = erase_block(ERASE_BLK_1);
      break;
    case '2':
      err = erase_block(ERASE_BLK_2);
      break;
    case '3':
      err = erase_block(ERASE_BLK_3);
      break;
    case '4':
      err = erase_block(ERASE_BLK_4);
      break;
  }

  return err;
}

Error program_data()
{
  byte program_data[128] = { 0 };

  remove_unused_serial_char();
  Serial.println("Enter your hex data: ");
  while(!Serial.available()); // Wait for the answer

  String input = Serial.readStringUntil('\n');
  input.trim();

  Error err;
  int byteCount = 0;
  // Process each pair of hex characters
  for (int i = 0; i < input.length(); i++) {

    if (0 != i && ':' == input[i])
    {
      Serial.print("Data to write is : ");
      Serial.write(program_data, byteCount);
      Serial.println("");
      err = write_to_mcu(program_data, byteCount);
      CHECK_ERROR_WITH_RETURN(err, "program_data - write_to_mcu");
      byteCount = 0;
    }
    program_data[byteCount++] = input[i];
  
    if (byteCount >= sizeof(program_data)) break;
  }

  Serial.print("Data to write is : ");
  Serial.write(program_data, byteCount);
  Serial.println("");

  return write_to_mcu(program_data, byteCount);
}

int insert_display_memory()
{
  int value = 0;
  while(!Serial.available()); // Wait for the answer
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n'); // read until Enter key
    input.trim(); // remove spaces or \r

    // convert the string (like "2E") to an integer in base 16
    value = strtol(input.c_str(), NULL, 16);
    Serial.print("You entered: 0x");
    Serial.println(value, HEX);
  }

  return value;
}

Error display_memory()
{
  remove_unused_serial_char();
  Serial.println("Enter your start address (2 bytes - in hex (e.g. 2E for 0x2E))");
  Serial.println("MSB:");

  byte start_addr[2] = { 0 }; // TODO: Remove 2 magic numbers and in the loop
  byte end_addr[2] = { 0 };

  start_addr[0] = (byte)insert_display_memory();
  Serial.println("LSB:");
  start_addr[1] = (byte)insert_display_memory();

  Serial.println("End address - MSB:");
  end_addr[0] = (byte)insert_display_memory();
  Serial.println("LSB:");
  end_addr[1] = (byte)insert_display_memory();

  return display_memory(start_addr, end_addr);
}



void setup ()
{
  mcuSerial.begin(BAUDRATE);
  Serial.begin(BAUDRATE);

  delay_without_stop(DELAY_BOOTTIME);
  enterBootloader();

  /* MCU entered in the bootloader stage. 
  We have to send the character "U" and wait for its response. */
  Error err = initialize_baudrate();
  CHECK_ERROR(err, "Initialize baudrate");

  Serial.println("Welcome to the AT89C51RB2 flash program: ");
}

void loop ()
{
  Serial.println("---------------------------------------");
  Serial.println("Read manufacturer ID: 0");
  Serial.println("Read SSB: 1");
  Serial.println("Full Chip Erase: 2");
  Serial.println("Erase a memory block: 3");
  Serial.println("Program data: 4");
  Serial.println("Display memory data: 5");
  Serial.println("Finish flash: 6");
  Serial.println("Read Hardware Byte: 7");
  while(!Serial.available()); // Wait for an answer

  Error err = OK;
  int choice = read_answer();

  switch (choice)
  {
    case '0':
      err = read_mcu_id();
      break;
    case '1':
      err = read_SSB();
      break;
    case '2':
      err = full_chip_erase();
      break;
    case '3':
      err = erase_blocks();
      break;

    case '4':
      err = program_data();
      break;

    case '5':
      err = display_memory();
      break;
    
    case '6':
      err = finish_flash();
      break;
    
    case '7':
      err = read_bytes();
      break;
  }
  CHECK_ERROR(err, "error");

  while(Serial.available()) { Serial.read(); } // Empty the buffer.
}  
