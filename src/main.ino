#include "drivers/computer_serial.h"
#include "drivers/mcu_serial.h"
#include "protocol/at89c51rb2_isp.h"
#include "cli/cli.h"

//void remove_unused_serial_char()
//{
  //while(Serial.available()) { Serial.read(); } // Read remaining unused characters.
//}

//byte read_answer()
//{
  //byte b = Serial.read();
  //Serial.readStringUntil('\n'); // Remove '\n' carriage
  //return b;
//}

////Error erase_blocks()
////{
  ////Error err = OK;
  ////remove_unused_serial_char();
  ////Serial.println("Block 0, 1, 2, 3, 4: ");
  ////while(!Serial.available()); // Wait for the answer
  ////int second_choice = read_answer();
  ////switch (second_choice)
  ////{
    ////case '0':
      ////err = erase_block(ERASE_BLK_0);
      ////break;
    ////case '1':
      ////err = erase_block(ERASE_BLK_1);
      ////break;
    ////case '2':
      ////err = erase_block(ERASE_BLK_2);
      ////break;
    ////case '3':
      ////err = erase_block(ERASE_BLK_3);
      ////break;
    ////case '4':
      ////err = erase_block(ERASE_BLK_4);
      ////break;
  ////}

  ////return err;
////}

//Error program_data()
//{
  //Error err = ERR_OK;
  //byte program_data[128] = { 0 };

  //remove_unused_serial_char();
  //Serial.println("Enter your hex data: ");
  //while(!Serial.available()); // Wait for the answer

  //String input = Serial.readStringUntil('\n');
  //input.trim();


  //int offset = 0;
  //do 
  //{
    ///* Get the beginning of the data */
    //byte begin_byte = input[offset + 0];
    //if (':' != begin_byte)
    //{
      //Serial.println("The beginning of the buffer is not ':'. Error.");
      //err = ERR_OK; 
      //break;
    //}
    ///* Get the data size */
    //byte data_size[2];
    //memcpy(data_size, &input[offset + 1], sizeof(data_size));
    //Serial.print("Data size: ");
    //Serial.print((char)data_size[0]);
    //Serial.print(" ");
    //Serial.println((char)data_size[1]);
    //uint8_t converted_size = hex_to_byte(data_size);
    //Serial.print("DAta size converted: ");
    //Serial.println(converted_size);

    ///* Get the address */
    //byte address[4];
    //memcpy(address, &input[offset + 3], sizeof(address));
    //Serial.print("Address : ");
    //Serial.print((char)address[0]);
    //Serial.print((char)address[1]);
    //Serial.print((char)address[2]);
    //Serial.println((char)address[3]);
    //uint8_t msb_address = hex_to_byte(address);
    //uint8_t lsb_address = hex_to_byte(&address[2]);
    //uint16_t converted_address = ((msb_address << 8) | lsb_address);
    //Serial.print("converted address = ");
    //Serial.println(converted_address);


    ///* Get the data */
    //byte data[255];
    //for (int i = 0; i < converted_size * 2; i++)
    //{
      //data[i] = input[offset + 9 + i]; // Skip the header
    //}

    //Serial.print("Data: ");
    ///* Check the size */
    //for (int i = 0; i < converted_size * 2; i++)
    //{        
      
      //Serial.print((char)data[i]);
      //Serial.print(" ");
      //if (':' == data[offset + i])
      //{
        //Serial.println("The size is incorrect");
        //Serial.println(converted_size);
        //Serial.println(offset);
        //Serial.println(data[offset + i]);
        //err = ERR_OK;
        //break;
      //}
    //}

    //if (ERR_OK == err)
    //{
      //err = write_program_data(data, converted_size, converted_address);
    //}


    //offset += (9 + converted_size * 2 + 2); // 9 header + 2 checksum 
    //Serial.print("Offset == ");
    //Serial.println(offset);
    //Serial.print("input length == ");
    //Serial.println(input.length());
  //} while (ERR_OK == err && offset < input.length());
  
  //if (ERR_OK != err)
  //{
    //Serial.println("Exit loop with error");
  //}
  
  //// // Process each pair of hex characters
  //// for (int i = 0; i < input.length(); i++) {

  ////   if (0 != i && ':' == input[i])
  ////   {
  ////     Serial.print("Data to write is : ");
  ////     Serial.write(program_data, byteCount);
  ////     Serial.println("");
  ////     err = write_program_data(program_data, byteCount);
  ////     CHECK_ERROR_WITH_RETURN(err, "program_data - write_to_mcu");
  ////     byteCount = 0;
  ////   }
  ////   if (index_command > 9) // header
  ////   {
  ////     program_data[byteCount - 9] = input[i];
  ////   }
    
  ////   byteCount++;
  ////   if (byteCount >= sizeof(program_data)) break;
  //// }

  //// Serial.print("Data to write is : ");
  //// Serial.write(program_data, byteCount);
  //// Serial.println("");

  //return err; //write_to_mcu(program_data, byteCount);
//}

//int insert_display_memory()
//{
  //int value = 0;
  //while(!Serial.available()); // Wait for the answer
  //if (Serial.available()) {
    //String input = Serial.readStringUntil('\n'); // read until Enter key
    //input.trim(); // remove spaces or \r

    //// convert the string (like "2E") to an integer in base 16
    //value = strtol(input.c_str(), NULL, 16);
    //Serial.print("You entered: 0x");
    //Serial.println(value, HEX);
  //}

  //return value;
//}

//Error display_memory()
//{
  //remove_unused_serial_char();
  //Serial.println("Enter your start address (2 bytes - in hex (e.g. 2E for 0x2E))");
  //Serial.println("MSB:");

  //byte start_addr[2] = { 0 }; // TODO: Remove 2 magic numbers and in the loop
  //byte end_addr[2] = { 0 };

  //start_addr[0] = (byte)insert_display_memory();
  //Serial.println("LSB:");
  //start_addr[1] = (byte)insert_display_memory();

  //Serial.println("End address - MSB:");
  //end_addr[0] = (byte)insert_display_memory();
  //Serial.println("LSB:");
  //end_addr[1] = (byte)insert_display_memory();

  //return display_memory(start_addr, end_addr);
//}


void setup ()
{
  ////mcuSerial.begin(BAUDRATE);
  //Serial.begin(19200);

  ////delay_without_stop(DELAY_BOOTTIME);
  ////enterBootloader();
  //Error err = at89c51rb2.enter_bootloader();
  //if (ERR_OK != err)
  //{
    //Serial.println("Failed to initialize bootloader");
    //exit(0);
  //}

  ///* MCU entered in the bootloader stage. 
  //We have to send the character "U" and wait for its response. */
  ////Error err = initialize_baudrate();
  ////CHECK_ERROR(err, "Initialize baudrate");

  //Serial.println("Welcome to the AT89C51RB2 flash program: ");

  computer_serial_init();
  computer_serial_print("Computer communication initialized\n");
  mcu_serial_init();
  computer_serial_print("MCU communication initialized\n");
  if (0 != at89c51rb2_enter_bootloader())
  {
    computer_serial_print("Failed to enter bootloader\n");
    delay(1000);
    exit(0);
  }
  computer_serial_print("Bootloader enabled\n");
  cli_init();
}

void loop ()
{
  cli_task();
  //Serial.println("---------------------------------------");
  //Serial.println("Read manufacturer ID: 0");
  //Serial.println("Read SSB: 1");
  //Serial.println("Full Chip Erase: 2");
  //Serial.println("Erase a memory block: 3");
  //Serial.println("Program data: 4");
  //Serial.println("Display memory data: 5");
  //Serial.println("Finish flash: 6");
  //Serial.println("Read Hardware Byte: 7");
  //while(!Serial.available()); // Wait for an answer

  //Error err = ERR_OK;
  //int choice = read_answer();

  //switch (choice)
  //{
    //case '0':
      ////err = read_mcu_id();
      //break;
    //case '1':
      ////err = read_SSB();
      //break;
    //case '2':
      ////err = full_chip_erase();
      //break;
    //case '3':
      ////err = erase_blocks();
      //break;

    //case '4':
      //err = program_data();
      //break;

    //case '5':
      ////err = display_memory();
      //break;
    
    //case '6':
      ////err = finish_flash();
      //break;
    
    //case '7':
      ////err = read_bytes();
      //break;
  //}
  //CHECK_ERROR(err, "error");

  //while(Serial.available()) { Serial.read(); } // Empty the buffer.
}  
