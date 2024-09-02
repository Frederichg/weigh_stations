#include <Arduino.h>

/*
  Reading multiple RFID tags, simultaneously!
  By: Nathan Seidle @ SparkFun Electronics
  Date: October 3rd, 2016
  https://github.com/sparkfun/Simultaneous_RFID_Tag_Reader

  Single shot read - Ask the reader to tell us what tags it currently sees. And it beeps!

  If using the Simultaneous RFID Tag Reader (SRTR) shield, make sure the serial slide
  switch is in the 'SW-UART' position.
*/

// Library for controlling the RFID module
#include "Arduino.h"
#include "SparkFun_UHF_RFID_Reader.h"

// Create instance of the RFID module
RFID rfidModule;

// By default, this example assumes software serial. If your platform does not
// support software serial, you can use hardware serial by commenting out these
// lines and changing the rfidSerial definition below
#include <SoftwareSerial.h>
SoftwareSerial softSerial(2, 3); //RX, TX

// Here you can specify which serial port the RFID module is connected to. This
// will be different on most platforms, so check what is needed for yours and
// adjust the definition as needed. Some examples are provided below
#define rfidSerial softSerial // Software serial (eg. Arudino Uno or SparkFun RedBoard)
// #define rfidSerial Serial1 // Hardware serial (eg. ESP32 or Teensy)

// Here you can select the baud rate for the module. 38400 is recommended if
// using software serial, and 115200 if using hardware serial.
#define rfidBaud 38400
// #define rfidBaud 115200

// Here you can select which module you are using. This library was originally
// written for the M6E Nano only, and that is the default if the module is not
// specified. Support for the M7E Hecto has since been added, which can be
// selected below
#define moduleType ThingMagic_M6E_NANO
// #define moduleType ThingMagic_M7E_HECTO

#define BUZZER1 9
//#define BUZZER1 0 //For testing quietly
#define BUZZER2 10


void write();
void read(char);
boolean setupRfidModule(long);

void setup()
{
  Serial.begin(115200);

  pinMode(BUZZER1, OUTPUT);
  pinMode(BUZZER2, OUTPUT);

  digitalWrite(BUZZER2, LOW); //Pull half the buzzer to ground and drive the other half.

  while (!Serial);
  Serial.println();
  Serial.println("Initializing...");

  if (setupRfidModule(rfidBaud) == false)
  {
    Serial.println("Module failed to respond. Please check wiring.");
    while (1); //Freeze!
  }

  rfidModule.setRegion(REGION_NORTHAMERICA); //Set to North America

  rfidModule.setReadPower(2700); //5.00 dBm. Higher values may cause USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling
  rfidModule.setWritePower(2700); //5.00 dBm. Higher values may cause USB port to brown out
  //Max Write TX Power is 27.00 dBm and may cause temperature-limit throttling
}

void loop()
{
  Serial.println(F("r to read and w to write"));
  while (!Serial.available()); //Wait for user to send a character
  char chr = char(Serial.read());
  if (chr == 'r') {read('r');}
  if (chr == 'h') {read('h');}
  else if (chr == 'w') {write();}    
}

void write() {

  Serial.println(F("input EPC"));
  while (!Serial.available());
  String input = Serial.readString();

  char stringEPC[12];
  for (int x = 0; x < input.length(); x++){
    if (x >= 12) {break;}
    stringEPC[x] = input[x];
  }
  for (int x = input.length(); x < 12; x++){
    stringEPC[x] = 0x00;
  }

  byte responseTypeEPC = rfidModule.writeTagEPC(stringEPC, 12); //The -1 shaves off the \0 found at the end of string

  Serial.println(F("input UD"));
  while (!Serial.available());
  input = Serial.readString();

  char stringUD[12];
  for (int x = 0; x < input.length(); x++){
    if (x >= 12) {break;}
    stringUD[x] = input[x];
  }
  for (int x = input.length(); x < 12; x++){
    stringUD[x] = 0x00;
  }

  byte responseTypeUD = rfidModule.writeUserData(stringUD, 12); //The -1 shaves off the \0 found at the end of string

  // char hexEPC[] = str; //You can only write even number of bytes
  // byte responseType = rfidModule.writeTagEPC(hexEPC, sizeof(hexEPC));

  if (responseTypeEPC == RESPONSE_SUCCESS && responseTypeUD == RESPONSE_SUCCESS){
    Serial.println("New EPC Written!\n");
      //Beep! Piano keys to frequencies: http://www.sengpielaudio.com/KeyboardAndFrequencies.gif
    tone(BUZZER1, 2093, 150); //C
    delay(150);
    tone(BUZZER1, 2349, 150); //D
    delay(150);
    tone(BUZZER1, 2637, 150); //E
    delay(150);
  }
  else{
    Serial.println("Failed write");
    tone(BUZZER1, 2637, 150); //E
    delay(300);
    tone(BUZZER1, 2637, 150); //D
    delay(300);
    tone(BUZZER1, 2637, 150); //C
    delay(300);
  }
}

void read(char mode) {
  byte myEPC[12];
  byte myEPClength;
  byte myUD[12];
  byte myUDlength;
  byte responseType = 0;

  Serial.print(F("Searching for tag"));
  while (responseType != RESPONSE_SUCCESS)//RESPONSE_IS_TAGFOUND)
  {
    myEPClength = sizeof(myEPC); //Length of EPC is modified each time .readTagEPC is called

    responseType = rfidModule.readTagEPC(myEPC, myEPClength, 500); //Scan for a new tag up to 500ms
    Serial.print(F("."));
  }
  responseType = 0;
  while (responseType != RESPONSE_SUCCESS)//RESPONSE_IS_TAGFOUND)
  {
    myUDlength = sizeof(myUD); //Length of EPC is modified each time .readTagEPC is called

    responseType = rfidModule.readUserData(myUD, myUDlength, 500); //Scan for a new tag up to 500ms
    Serial.print(F("."));
  }
  tone(BUZZER1, 2093, 150); //C
  delay(150);
  tone(BUZZER1, 2637, 150); //E
  delay(150);
  tone(BUZZER1, 2349, 150); //D
  delay(150);
  Serial.println();

  if (mode == 'r')
  {    //Print EPC
    Serial.print("EPC : ");
    Serial.println(atoi(myEPC));
    Serial.print("UD : ");
    Serial.println(atoi(myUD));
  }
  else{
    Serial.print(F(" epc["));
    for (byte x = 0 ; x < myEPClength ; x++)
    {
      if (myEPC[x] < 0x10) Serial.print(F("0"));
      Serial.print(myEPC[x], HEX);
      Serial.print(F(" "));
    }
    Serial.println(F("]"));

    Serial.print(F(" UD["));
    for (byte x = 0 ; x < myUDlength ; x++)
    {
      if (myUD[x] < 0x10) Serial.print(F("0"));
      Serial.print(myUD[x], HEX);
      Serial.print(F(" "));
    }
    Serial.println(F("]"));
  }
  Serial.print("\n");
}

//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupRfidModule(long baudRate)
{
  rfidModule.begin(rfidSerial, moduleType); //Tell the library to communicate over serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  rfidSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  delay(100); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while (rfidSerial.available())
    rfidSerial.read();

  rfidModule.getVersion();

  if (rfidModule.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    rfidModule.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    rfidSerial.begin(115200); //Start serial at 115200

    rfidModule.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    rfidSerial.begin(baudRate); //Start the serial port, this time at user's chosen baud rate

    delay(250);
  }

  //Test the connection
  rfidModule.getVersion();
  if (rfidModule.msg[0] != ALL_GOOD)
    return false; //Something is not right

  //The module has these settings no matter what
  rfidModule.setTagProtocol(); //Set protocol to GEN2

  rfidModule.setAntennaPort(); //Set TX/RX antenna ports to 1

  return true; //We are ready to rock
}
