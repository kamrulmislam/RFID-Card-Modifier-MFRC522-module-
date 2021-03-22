#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key old_key;
MFRC522::MIFARE_Key new_key;

MFRC522::StatusCode status;

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card


  // Prepare the key (used both as key A and as key B)
  // FF FF FF FF FF FF is the default key

  old_key.keyByte[0] = 0xFF;
  old_key.keyByte[1] = 0xFF;
  old_key.keyByte[2] = 0xFF;
  old_key.keyByte[3] = 0xFF;
  old_key.keyByte[4] = 0xFF;
  old_key.keyByte[5] = 0xFF;

  new_key.keyByte[0] = 0x00;  // write your new key here
  new_key.keyByte[1] = 0x00;
  new_key.keyByte[2] = 0x00;
  new_key.keyByte[3] = 0x00;
  new_key.keyByte[4] = 0x00;
  new_key.keyByte[5] = 0x00;

  Serial.println();
  Serial.println();
  Serial.println(F("BEWARE: Key will be changed..........."));
  Serial.println();
}

/**
   Main loop.
*/
void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  Serial.println();

  // Check for compatibility
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }

  byte dataBlock[]    = {
    0x00, 0x00, 0x00, 0x00, //  First 6 bytes for key A
    0x00, 0x00, 0xFF, 0x07, //  Last 6 bytes for key B
    0x80, 0x69, 0x00, 0x00, //  Don't touch the middle 4 bytes FF 07 80 69
    0x00, 0x00, 0x00, 0x00
  };

  // Key Change
  for (byte sector = 0; sector < 16; sector++) {
    byte blockAddr = sector * 4 + 3;              // to change key blockAddr will always be (sector*4 + 3)
    change_key(sector, blockAddr, dataBlock);
  }


  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

}

//Helper routine to dump a byte array as hex values to Serial.

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

//key Change

void change_key(byte sector, byte blockAddr, byte *dataBlock) {
  byte trailerBlock   = 3; // this block is needed to authenticate the card. It'll always be 3.

  // Authenticate using key A
  if (sector == 0) {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &old_key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
  } else {
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &new_key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return;
    }
  }

  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &old_key, sector);
  Serial.println();

  // Write data to the block
  Serial.print(F("Writing data into block ")); Serial.print(blockAddr);
  Serial.println(F(" ..."));
  dump_byte_array(dataBlock, 16); Serial.println();
  status = mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();

}
