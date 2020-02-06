/**
   --------------------------------------------------------------------------------------------------------------------
   Example sketch/program showing how to read data from more than one PICC to serial.
   --------------------------------------------------------------------------------------------------------------------
   This is a MFRC522 library example; for further details and other examples see: https://github.com/miguelbalboa/rfid

   Example sketch/program showing how to read data from more than one PICC (that is: a RFID Tag or Card) using a
   MFRC522 based RFID Reader on the Arduino SPI interface.

   Warning: This may not work! Multiple devices at one SPI are difficult and cause many trouble!! Engineering skill
            and knowledge are required!

   @license Released into the public domain.

   Typical pin layout used:
   -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS 1    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required *
   SPI SS 2    SDA(SS)      ** custom, take a unused pin, only HIGH/LOW required *
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15

*/

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN 4// On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 12 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many Pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel Pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define DELAYVAL 10 // Time (in milliseconds) to pause between Pixels

#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_1_PIN        10         // Configurable, take a unused pin, only HIGH/LOW required, must be diffrent to SS 2
#define SS_2_PIN        8
#define SS_3_PIN        7
#define SS_4_PIN        6
char c;

#define NO_OF_READERS   4

byte ssPins[] = {
  SS_1_PIN,
  SS_2_PIN,
  SS_3_PIN,
  SS_4_PIN
};
byte tags[NO_OF_READERS];
byte prev_state = 0;
byte curr_state = 0;
String detected_toy = "\0";
String correct_toy = "\0";

MFRC522 mfrc522[NO_OF_READERS];   // Create MFRC522 instance.

byte bluetoothTx = 2; // D2 to TXD
byte bluetoothRx = 3; // D3 to RXD  (Warning! See Text)

SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

void neopixel() {

  Pixels.clear(); // Set all pixel colors to 'off'

  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of Pixels minus one.

  for (int j = 0; j < NUMPIXELS; j++) { // For each pixel...
    // Pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    Pixels.setPixelColor(j, Pixels.Color(0, 150, 0));
    Pixels.show();   // Send the updated pixel colors to the hardware.
    delay(DELAYVAL); // Pause before next pass through loop
  }

  for (int j = 0; j < NUMPIXELS; j++) { // For each pixel...
    // Pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
    // Here we're using a moderately bright green color:
    Pixels.setPixelColor(j, Pixels.Color(0, 0, 0));
    Pixels.show();   // Send the updated pixel colors to the hardware.
    delay(DELAYVAL); // Pause before next pass through loop
  }
}

byte dump_byte_array(byte *buffer, byte bufferSize) {

  byte tag = '\0';
  for (byte i = 0; i < bufferSize; i++)
    tag = tag + char(buffer[i]);
  return tag;
}

void setup() {

  Serial.begin(115200); // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  Pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  bluetooth.begin(9600);

  for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
}

void loop() {
  if (bluetooth.available()) {
    correct_toy = "\0";
    while (bluetooth.available()) {
      c =bluetooth.read();
      if (c == '#') {
        break;
      }
      correct_toy += c;
    }
  }
  correct_toy.trim();
  if (correct_toy.length()>0) {
    Serial.println(correct_toy.length());
    bluetooth.println(correct_toy.length());
    bluetooth.println(correct_toy);
  }
//  if (correct_toy=="CAT")
//    {
//    neopixel();
//    Serial.println(correct_toy);


//  while(!bluetooth.available()&&correct_toy=='\0');

//  correct_toy = (char)bluetooth.read();
//  Serial.println(correct_toy);
//  bluetooth.println(correct_toy);
//  correct_toy = "A";
  prev_state = curr_state;
  for (uint8_t reader = 0; reader < NO_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init();
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      tags[reader] = dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);


      // Halt PICC
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD
      mfrc522[reader].PCD_StopCrypto1();
    }
    //if (mfrc522[reader].PICC_IsNewC
    else {
      tags[reader] = 0;
    }
    curr_state = tags[0] + tags[1] + tags[2] + tags[3];
  }
  //for(uint8_t reader
//  Serial.print("Current State:");
//  Serial.print(curr_state);
  // delay(100);
  if (prev_state != curr_state) {
    if ((curr_state - prev_state) == 3 || (curr_state - prev_state) == -3 || curr_state == 3) {
      detected_toy =String("CAT");
//      neopixel();
    }
    else if ((curr_state - prev_state) == 33 || (curr_state - prev_state) == -33 || curr_state == 33) {
      detected_toy =String("DOG");
//      neopixel();
    }
    else if ((curr_state - prev_state) == 140 || (curr_state - prev_state) == -140 || curr_state == 140) {
      detected_toy =String("DUCK");
//     neopixel();
    }

  }

  if (curr_state < prev_state)
  {
    detected_toy = "TOY_REMOVED";
//    Pixels.clear();
  }
    Serial.print("Detected toy:");
  Serial.println(detected_toy);
     Serial.print("Correct toy:");
    Serial.println(correct_toy);
//  Serial.print("\t");
//  Serial.println(correct_toy);
  if (detected_toy == correct_toy) {
    Serial.print("Correct toy Check: ");
    Serial.println(correct_toy);
    neopixel();
    bluetooth.println("TRUE");
  }


//  bluetooth.println("FALSE");
  delay(50);
}
