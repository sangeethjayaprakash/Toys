#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#define RST_PIN 9
#define NO_OF_READERS   2
int interruptPins[NO_OF_READERS] = {2,3};
int ssPins[NO_OF_READERS] = {4,5};
volatile bool interrups[NO_OF_READERS] = {false, false};
volatile bool oldState[NO_OF_READERS] = {true, true};
volatile bool newState[NO_OF_READERS] = {true, true};
MFRC522 mfrc522[NO_OF_READERS];
byte regVal = 0x7F;

/*
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg, mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg, mfrc522.PCD_Transceive);
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg, 0x7F);
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void isr() {
  for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
    newState[reader] = digitalRead(interruptPins[reader]);
    if (oldState[reader] != newState[reader]) interrups[reader] = true;
    oldState[reader] = newState[reader];
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();          // Init SPI bus

  Serial.print("Reading tags...");

  regVal = 0xA0; //rx irq
  for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    mfrc522[reader].PCD_WriteRegister(mfrc522[reader].ComIEnReg, regVal);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
    pinMode(interruptPins[reader], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(interruptPins[reader]), isr, FALLING);
  }
}

void loop() {
  for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
    if(interrups[reader]) {
      Serial.print(F("Interrupt. "));
      mfrc522[reader].PICC_ReadCardSerial(); //read the tag data
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      clearInt(mfrc522[reader]);
      mfrc522[reader].PICC_HaltA();
      interrups[reader] = false;
    }
    activateRec(mfrc522[reader]);
  }
}












// void readTag() {
//   if()
// }
//
// void setup() {
//   Serial.begin(9600);
//   while (!Serial);      // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
//   SPI.begin();          // Init SPI bus
//
//   Serial.print("Reading tags...");
//   for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
//     mfrc522[reader].PCD_Init(sdaPin[reader], RST_PIN); // Init each MFRC522 card
//     mfrc522[reader].PCD_WriteRegister(mfrc522[reader].ComIEnReg, regVal);
//     Serial.print(F("Reader "));
//     Serial.print(reader);
//     Serial.print(F(": "));
//     mfrc522[reader].PCD_DumpVersionToSerial();
//     pinMode(readerPin[reader], INPUT_PULLUP);
//     PCintPort::attachInterrupt(readerInterrruptPin[reader], readTag, CHANGE);
//   }
// }
//
// void loop() {
//   for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
//     Serial.print(tag[reader] + '\t');
//   }
// }
