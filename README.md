# RFID-Card-Modifier
This repo will help you to modify, clone, read/write RFID card


1. Install Arduino IDE.
2. Build the circuit.
     Module: RC522, Arduino Uno
  
  
            (SDA  => Digital 10)
            (SCK  => Digital 13)
            (MOSI =>	Digital 11)
            (MISO =>	Digital 12
            (IRQ  => No need to connect)
            (GND  =>	GND)
            (RST  =>	Digital 9)
            (3.3V =>	3.3V (must be 3.3V))
            
   Check the "circuit.png" file for details.
   [Ref: https://randomnerdtutorials.com/security-access-using-mfrc522-rfid-reader-with-arduino/]
 
 3. Download "MFRC522" library for arduino
 4. Run the scripts on arduino IDE and modify RFID cards
      
