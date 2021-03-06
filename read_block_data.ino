#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;

MFRC522::StatusCode status;

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card

  // Prepare the key (used both as key A and as key B)
  // FF FF FF FF FF FF is the default key

  key.keyByte[0] = 0xFF;   // write your card key here
  key.keyByte[1] = 0xFF;
  key.keyByte[2] = 0xFF;
  key.keyByte[3] = 0xFF;
  key.keyByte[4] = 0xFF;
  key.keyByte[5] = 0xFF;

  Serial.println();
  Serial.println();
  Serial.println(F("Scan a RFID Card: "));
  Serial.print(F("Using key:"));
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
  Serial.println();
}

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

  // sector number as first parameter => [0 to 15]
  // block number as second parameter => [0 to 63] which also depends on sector number.
  // for sector 0 readable blocks will be 0 to 3, for sector 1 readable blocks will be 4 to 7
  read_sector_data(0, 1);


  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();

}

//Helper to dump a byte array as hex values to Serial.
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

//Helper to convert hex values as asci
void hex2ascii(byte *buffer, byte bufferSize) {
  for (uint8_t i = 0; i < bufferSize; i++)
  {
    if (buffer[i] != 32)
    {
      Serial.write(buffer[i]);
    }
  }

}

void read_sector_data(byte sector, byte block) {
  byte trailerBlock   = 3; // this block is needed to authenticate the card. It'll always be 3.
  byte blockAddr = block;
  byte buffer[18]; // Values will be stored in this variable
  byte size = sizeof(buffer);


  // Authenticate using key A
  Serial.println(F("Authenticating using key A..."));
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }


  // Show the whole sector as it currently is
  Serial.println(F("Data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // Read data from the block
  status = mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Read() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.print(F("Data in block ")); Serial.print(blockAddr); Serial.println(F(":"));
    Serial.print(F("Hex Format: ")); dump_byte_array(buffer, 16); Serial.println();
    Serial.print(F("Ascii Format: ")); hex2ascii(buffer, 16); Serial.println();
  }
}
