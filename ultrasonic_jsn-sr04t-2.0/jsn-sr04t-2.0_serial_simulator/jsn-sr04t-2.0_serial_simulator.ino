#include <HardwareSerial.h>
#include <U8g2lib.h> // OLED Universal Graphics library for SH1106 display chip

//
// simulate two ultra sonic sensor with serial interface
// 

// write serial1 simualtor
#define rxPin1 25 // esp32 hardware 1rx
#define txPin1 26 // esp32 hardware 1tx

// write serial2 simulator
#define rxPin2 16 // esp32 hardware 2rx
#define txPin2 17 // esp32 hardware 2tx

bool enableSerial1 = false;
bool enableSerial2 = true;

// pins for OLED display
#define sdaPin 21 // Display: SDA Pin
#define sclPin 22 // Display: SCL Pin

/* **************** DISPLAY SETTINGS **************** */
#define ROW_HEIGHT 9
#define COL_WIDTH 16
#define LEFT_PIXEL_ROW 0

// U8G2_ST7920_128X64_1_SW_SPI u8g2(rotation, clock, data, cs [, reset])
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // constructor for the particular OLED
#define OLED_ADDRESS 0x3C

// HarwareSerial K02(16, 17); // RX, TX
// HardwareSerial Serial2(2);


/* **********************************************************
/*                    Global Variables                      *
/* ******************************************************** */
int8_t i;
int8_t j;
int16_t k;

String inputString = "";
String inputStringDisplay = "";
String serialInfoDisplay = "";
String errorInfoDisplay = "";

bool getTriggerByte1 = false;
bool getTriggerByte1Display = false;
bool getWrongByte1 = false;
bool getWrongByte1Display = false;

bool getTriggerByte2 = false;
bool getTriggerByte2Display = false;
bool getWrongByte2 = false;
bool getWrongByte2Display = false;

#define LONG_WAIT_TIME 10000
#define INFO_WAIT_TIME 1000
#define DISPLAY_WAIT_TIME 2500
unsigned long longtimer = millis() - LONG_WAIT_TIME;
unsigned long infotimer = millis() - INFO_WAIT_TIME;

unsigned long triggerbyte1timer = millis() - DISPLAY_WAIT_TIME;
unsigned long wrongbyte1timer = millis() - DISPLAY_WAIT_TIME;

unsigned long triggerbyte2timer = millis() - DISPLAY_WAIT_TIME;
unsigned long wrongbyte2timer = millis() - DISPLAY_WAIT_TIME;

unsigned long serialinfotimer = millis() - DISPLAY_WAIT_TIME;

int COUNTBACK = 0;
#define MAXRDN 19

byte verifybyte = 0x55;
byte inByte;

uint8_t  b[4];
uint8_t  crc[0];
uint16_t value;

/* **********************************************************
/*                       Void Setup                         *
/* ******************************************************** */

void setup()
{
  Serial.begin(115200);

  if (enableSerial1) {
    Serial1.begin(9600, SERIAL_8N1, rxPin1, txPin1);
    Serial1.begin(9600);  
  }
  
  if (enableSerial2) {
    Serial2.begin(9600, SERIAL_8N1, rxPin2, txPin2);
    Serial2.begin(9600);  
  }
  
  oled.begin();
  oled.clear();

  // display opening screen
  oled.setFont(u8g2_font_6x10_tf);
  oled.setDrawColor(1);

  k = 1000;
  
  delay(500);

}

/* **********************************************************
/*                      Void Loop                           *
/* ******************************************************** */
  
void loop()
{
  bool DEBUG = true;
  oled.clearBuffer();

  // Cycle to the next string every LONG_WAIT_TIME
  if (millis() > (longtimer + LONG_WAIT_TIME))
  {
    value = k++;

    int rdn9 = random(MAXRDN);

    // Serial.println("random number: ");
    // Serial.println(rdn9);

    b[0] = {0xFF};

    if (rdn9 == 0) {
      b[0] = {0xEE};
      errorInfoDisplay = "e0";
    }
      
    b[1] = *((uint8_t*)&(value)+1); //high byte (0x12)
    b[2] = *((uint8_t*)&(value)+0); //low byte  (0x34)
  
      // b[1] = {0x07};
      // b[2] = {0xA1};    
      // b[3] = {0xA7};
    
    if (rdn9 == 1) {
      b[1] = {0x07};
      errorInfoDisplay = "e1";
    }
    if (rdn9 == 2) {
      b[1] = {0xA1};
      errorInfoDisplay = "e2";
    }
  
      b[3] = (b[0] + b[1] + b[2]) & 0xFF;

    if (rdn9 == 3) {
      b[1] = {0x33};
      errorInfoDisplay = "e3";
    }

    if (rdn9 > 3) {
      errorInfoDisplay = "";
    }    
         
      Serial.print(value);
      Serial.print(" --> would send >>> ");
      for (i = 0; i < sizeof(b); i++) {
           printHexToSerial(b[i], Serial);
      }
      Serial.print("           ");
      Serial.print(errorInfoDisplay);
      Serial.println("");
  
      // uint8_t btest[] = {0xFF, 0x07, 0xA1, 0xA7};
      // uint8_t btest[] = {b[0], b[1], b[2], b[3]};
  
      // startByte + h_data + l_data)&0xFF
      // crc[0] = (btest[0] + btest[1] + btest[2]) & 0xFF;
      // crc[0] = (b[0] + b[1] + b[2]) & 0xFF;
  
      // Serial.print("Checksum: ");
      // Serial.println(convertToHex( b[3] ));

    // set longtimer helper
    longtimer = millis();
    // reset CountBack
    COUNTBACK = LONG_WAIT_TIME/1000;  
  }  

  // Cycle to the next string every INFO_WAIT_TIME
  if (millis() > (infotimer + INFO_WAIT_TIME))
  {
    // set infotimer helper
    infotimer = millis();      
    // reduce CountBack
    COUNTBACK = COUNTBACK - INFO_WAIT_TIME/1000;
  }
 
  if (getTriggerByte1) {
    delay(50);
    getTriggerByte1Display = true;
    triggerbyte1timer = millis();
    
    Serial1.write(b, sizeof(b));

    // Serial.println(" get 0x55 from serial read ");
    Serial.print(" dat 1 ");
    for (i = 0; i < sizeof(b); i++) {
       printHexToSerial(b[i], Serial);
    }

    Serial.println(" written.");
    // k++;
        
    // clear the string:
    inputStringDisplay = inputString;
    inputString = "";
    getTriggerByte1 = false;
  }

  if (getTriggerByte2) {
    delay(50);
    getTriggerByte2Display = true;
    triggerbyte2timer = millis();
    
    Serial2.write(b, sizeof(b));

    // Serial.println(" get 0x55 from serial read ");
    Serial.print(" dat 2 ");
    for (i = 0; i < sizeof(b); i++) {
       printHexToSerial(b[i], Serial);
    }

    Serial.println(" written.");
    // k++;
        
    // clear the string:
    inputStringDisplay = inputString;
    inputString = "";
    getTriggerByte2 = false;
  }

  if (millis() > (triggerbyte1timer + DISPLAY_WAIT_TIME))
  {
    // set triggerbytetimer helper
    // triggerbytetimer = millis();    
    getTriggerByte1Display = false;
    resetInputStringDisplay();    
  }

  if (millis() > (triggerbyte2timer + DISPLAY_WAIT_TIME))
  {
    // set triggerbytetimer helper
    // triggerbytetimer = millis();    
    getTriggerByte2Display = false;
    resetInputStringDisplay();    
  }

  if (getWrongByte1) {
    delay(50);
    getWrongByte1Display = true;
    wrongbyte1timer = millis();

    // Serial.println(inputString);

    // Serial.println("Verifybyte != 0x55");
    
    Serial.print(" err 1 ");
    for (i = 0; i < sizeof(b); i++) {
      printHexToSerial(b[i], Serial);
    }
    
    Serial.print("   <<< ");
    Serial.println(" error.");
    
    // clear the string:
    inputStringDisplay = inputString;
    inputString = "";
    getWrongByte1 = false;
  }

  if (getWrongByte2) {
    delay(50);
    getWrongByte2Display = true;
    wrongbyte2timer = millis();

    // Serial.println(inputString);

    // Serial.println("Verifybyte != 0x55");
    
    Serial.print(" err 2 ");
    for (i = 0; i < sizeof(b); i++) {
      printHexToSerial(b[i], Serial);
    }
    
    Serial.print("   <<< ");
    Serial.println(" error.");
    
    // clear the string:
    inputStringDisplay = inputString;
    inputString = "";
    getWrongByte2 = false;
  }

  if (millis() > (wrongbyte1timer + DISPLAY_WAIT_TIME))
  {
    // set triggerbytetimer helper
    // triggerbytetimer = millis();
    getWrongByte1Display = false;
    resetInputStringDisplay();
  }

    if (millis() > (wrongbyte2timer + DISPLAY_WAIT_TIME))
  {
    // set triggerbytetimer helper
    // triggerbytetimer = millis();
    getWrongByte2Display = false;
    resetInputStringDisplay();
  }

  if (millis() > (serialinfotimer + DISPLAY_WAIT_TIME))
  {
    serialInfoDisplay = "";
  }

  display_data();
  oled.sendBuffer();
  
}


/* **********************************************************
/*                 Trigger                                  *
/* ******************************************************** */

/* ********************************************************************************
/* SerialEvent occurs whenever a new data comes in the hardware serial RX. This   *
/*  routine is run between each time loop() runs, so using delay inside loop can  *
/*  delay response. Multiple bytes of data may be available.                      *
/*  Because of Serial2 is used, serialEvent2 is used here!                        *
/* ********************************************************************************/

void serialEvent1() {
  bool DEBUG = true;
  while (Serial1.available()) {

    j = 0;

    Serial.println("=========================================================");
    Serial.print("got serial data on serial1  -->  ");
    
    serialinfotimer = millis();
    serialInfoDisplay = "Data on Serial2";
    // byte message[1]={0xFF};

    // Serial.print("Current k: ");
    // Serial.print(k);
    // Serial.print(" (hex: ");
    // Serial.print(convertToHex(k));
    // Serial.println(")");

    // get the new byte:
    byte inByte = (byte)Serial1.read();

    Serial.print("inByte: ");
    Serial.println(convertToHex(inByte));
  
    // add it to the inputString:
    inputString += convertToHex(inByte);    
    // if the incoming byte is equal verifybyte, set a flag so the main loop can
    // do something about it:
    if (inByte == verifybyte) {
      getTriggerByte1 = true;
    }
    else
    {
      getWrongByte1 = true;
    }
  }
}

void serialEvent2() {
  bool DEBUG = true;
  while (Serial2.available()) {

    j = 0;

    Serial.println("=========================================================");
    Serial.print("got serial data on serial2  -->  ");
    
    serialinfotimer = millis();
    serialInfoDisplay = "Data on Serial2";
    // byte message[1]={0xFF};

    // Serial.print("Current k: ");
    // Serial.print(k);
    // Serial.print(" (hex: ");
    // Serial.print(convertToHex(k));
    // Serial.println(")");

    // get the new byte:
    byte inByte = (byte)Serial2.read();

    Serial.print("inByte: ");
    Serial.println(convertToHex(inByte));
  
    // add it to the inputString:
    inputString += convertToHex(inByte);    
    // if the incoming byte is equal verifybyte, set a flag so the main loop can
    // do something about it:
    if (inByte == verifybyte) {
      getTriggerByte2 = true;
    }
    else
    {
      getWrongByte2 = true;
    }
  }
}

/* **********************************************************
/*                 Functions and Subroutines                *
/* ******************************************************** */

void display_data() {

  // serial
    // serial info for get data
    printToOled(serialInfoDisplay, 2, 1);

    // current value k
    printIntToOled(value, 1, 4);
    // printToOled("mm", 1, 7);    

  // infotimer
    
    if (inputStringDisplay=="") {
     printIntToOled(COUNTBACK, 1, 1);
     printToOled("cur", 1, 2);    
    }
    else
    {
      printToOled("i", 1, 6);
      printIntToOled(COUNTBACK, 1, 1);
      printToOled("----", 1, 2);      
    }

  // column testing
  // for (i = 0; i < 9; i++) {
  //  printIntToOled(i+1, 6, i+1);
  //}
  
  // print data
    printToOled(inputStringDisplay, 7, 1);

    for (i = 0; i < sizeof(b); i++) {
        // printDataToOled(b[i], i, 6);
        printHexToOled(b[i], 5, i+1);
    }

  // trigger byte 1
    if (getTriggerByte1Display) {
      printToOled("t", 1, 7);
      printToOled("written", 5, 6);
      printToOled("V-byte == 0x55", 3, 1);
    }
      
  // wrong byte 1
    if (getWrongByte1Display) {
      printToOled("w", 1, 8);
      printToOled("FAILED", 6, 6);
      printToOled("V-byte != 0x55", 4, 1);
    } 

      // trigger byte 2
    if (getTriggerByte2Display) {
      printToOled("t", 1, 7);
      printToOled("written", 5, 6);
      printToOled("V-byte == 0x55", 3, 1);
    }
      
  // wrong byte 2
    if (getWrongByte2Display) {
      printToOled("w", 1, 8);
      printToOled("FAILED", 6, 6);
      printToOled("V-byte != 0x55", 4, 1);
    } 

  // print current error type
   printToOled(errorInfoDisplay, 2, 8);    
  
}

void resetInputStringDisplay() {
    if ((getTriggerByte1Display == false) && (getWrongByte1Display == false)) {
      inputStringDisplay = "";      
    }      
    if ((getTriggerByte2Display == false) && (getWrongByte2Display == false)) {
      inputStringDisplay = "";      
    }      
}

void printHexToSerial(uint8_t num, HardwareSerial localSerial) {
  // char hexCar[2];
  // sprintf(hexCar, "%02X", num);
  // localSerial.print(hexCar);  
  localSerial.print(convertToHex(num));    
}

String convertToHex(uint8_t num) {
  char hexCar[2];
  sprintf(hexCar, "%02X", num);
  return hexCar;
}

// print number to display position
void printHexToOled(uint8_t num, int row, int col) {
  oled.setCursor(LEFT_PIXEL_ROW + COL_WIDTH * (col - 1), row * ROW_HEIGHT);
  oled.print(convertToHex(num));
}

// print number to display position
void printIntToOled(int num, int row, int col) {
  oled.setCursor(LEFT_PIXEL_ROW + COL_WIDTH * (col - 1), row * ROW_HEIGHT);
  oled.print(num);
}

// print number to display position
void printToOled(uint8_t num, int row, int col) {
  oled.setCursor(LEFT_PIXEL_ROW + COL_WIDTH * (col - 1), row * ROW_HEIGHT);
  oled.print(num);
}

// print string to display position
void printToOled(String str, int row, int col) {
  oled.setCursor(LEFT_PIXEL_ROW + COL_WIDTH * (col - 1), row * ROW_HEIGHT);
  oled.print(str);
}
