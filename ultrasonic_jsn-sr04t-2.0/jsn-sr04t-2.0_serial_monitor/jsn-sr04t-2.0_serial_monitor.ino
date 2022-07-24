#include <HardwareSerial.h>
#include <U8g2lib.h> // OLED Universal Graphics library for SH1106 display chip

// 
// get data from serial ports and show it on OLED display
// 

#define rxPin1 25 // esp32 hardware 1rx
#define txPin1 26 // esp32 hardware 1tx

#define rxPin2 16 // esp32 hardware 2rx
#define txPin2 17 // esp32 hardware 2tx

#define sdaPin 21 // Display: SDA Pin
#define sclPin 22 // Display: SCL Pin

// HardwareSerial Serial(0);
// HardwareSerial Serial1(1);
// HardwareSerial Serial2(2);

unsigned int distanceCM1, distanceCM2, count;
unsigned int distanceProc1, distanceProc2;
unsigned int bar_y1, bar_y2;

#define maxRangeMM 2200

#define sizeX 128
#define sizeY 64
#define barStartX 78
#define barStartY 0
#define barEndY 64
#define barWidth 24

bool enableSerialLog = true;
bool enableExtendedSerialLog = false;
int enableSound = 0;
int visbileState = 1;

#define DAT 32
#define CLK 35
int COUNT = 0;
int COUNTBACK = 0;

#define LONG_WAIT_TIME 5000
#define INFO_WAIT_TIME 1000
#define DISPLAY_WAIT_TIME 2500
unsigned long longtimer = millis() - LONG_WAIT_TIME;
unsigned long infotimer = millis() - INFO_WAIT_TIME;
unsigned long triggerbytetimer = millis() - DISPLAY_WAIT_TIME;
unsigned long wrongbytetimer = millis() - DISPLAY_WAIT_TIME;
unsigned long getData1timer = millis() - LONG_WAIT_TIME;
unsigned long getData2timer = millis() - LONG_WAIT_TIME;

/* **************** DISPLAY SETTINGS **************** */
#define ROW_HEIGHT 9
#define COL_WIDTH 16
#define LEFT_PIXEL_ROW 0

// U8G2_ST7920_128X64_1_SW_SPI u8g2(rotation, clock, data, cs [, reset])
U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // constructor for the particular OLED
#define OLED_ADDRESS 0x3C

int8_t i;
int8_t j;
int16_t k;

String inputString = "";
String inputStringDisplay = "";
String serialInfoDisplay = "";
bool getTriggerByte = false;
bool getTriggerByteDisplay = false;
bool getWrongByte = false;
bool getWrongByteDisplay = false;
bool getSerial1Data = false;
bool getSerial2Data = false;
int noSerial1Data = 0;
int noSerial2Data = 0;
int distanceFromSerial1 = -1;
int distanceFromSerial2 = -1;

byte startByte = 0xFF;

/* **********************************************************
/*                       Void Setup                         *
/* ******************************************************** */

void setup(){

  // // wait until serial port opens for native USB devices
  // while (! Serial) {
  //  delay(1);
  //}

  Serial1.begin(9600, SERIAL_8N1, rxPin1, txPin1);
  Serial2.begin(9600, SERIAL_8N1, rxPin2, txPin2);

  Serial.begin(115200);
  Serial.println("setup done.");

  oled.begin();
  oled.clear();
  
    // display opening screen
  oled.setFont(u8g2_font_6x10_tf);
  oled.setDrawColor(1);

  delay(500);
}

/* **********************************************************
/*                      Void Loop                           *
/* ******************************************************** */

void loop(){

  oled.clearBuffer();
 
  // Cycle to the next string every one second
  if (millis() > (longtimer + LONG_WAIT_TIME))
  {
    
    // writeWrongDataToSerial(Serial2);

    triggerGetDataToSerial(Serial1, "s1 ");
    triggerGetDataToSerial(Serial2, "s2 ");

      if (millis() > (getData1timer + LONG_WAIT_TIME)) {
        noSerial1Data++;
      }
      if (millis() > (getData2timer + LONG_WAIT_TIME)) {
        noSerial2Data++;
      }
    
    // distanceCM1 = getDistanceSerialMm(Serial1, "1")/10;
    // distanceCM2 = getDistanceSerialMm(Serial2, "2")/10;
       
    // distanceCM1 = distanceCM2;

    if (visbileState) {
          oled.setCursor(10, 120);
          oled.println("*");
          oled.setDrawColor(2);
          visbileState = 0;
    }
    else
    {
      visbileState = 1;
    }

    // sevseg.setChars(testStrings[testStringsPos]);
    // testStringsPos++;
    // if (testStringsPos >= MAX_NUMBER_STRINGS) testStringsPos = 0;
    longtimer = millis();
    COUNTBACK = LONG_WAIT_TIME/1000;

  }


    // Cycle to the next string every one second
    if (millis() > (infotimer + INFO_WAIT_TIME))
    {      

      if (distanceFromSerial1<-1) {
        triggerGetDataToSerial(Serial1, "s1+");  
      }
      if (distanceFromSerial2<-1) {
        triggerGetDataToSerial(Serial2, "s2+");  
      }        
      
      if (enableExtendedSerialLog)
      {
        Serial.print("Infotimer: ");
        Serial.println(COUNTBACK);            
        Serial.print("Debug-Info: bar_y1=");
        Serial.print(bar_y1);
        Serial.print(" / bar_y2=");
        Serial.println(bar_y2);    
        Serial.print("Debug-NoData: noSerial1Data=");
        Serial.print(noSerial1Data);
        Serial.print(" / noSerial2Data=");
        Serial.println(noSerial2Data);    
      }    


      if (distanceCM1>0 || distanceCM2>0) 
      {
        if (enableSerialLog)
        {
          Serial.print("Distance 1 / 2 [cm]: ");
          Serial.print(distanceCM1);
          Serial.print(" [");
          Serial.print(noSerial1Data);          
          Serial.print("x no data] ");
          Serial.print(" / ");
          Serial.print(distanceCM2);
          Serial.print(" [");
          Serial.print(noSerial2Data);          
          Serial.println("x no data] ");
        }    

      }
      else
      {

        delay(100);
        if (count>=10) //to offen no data
        {
          Serial.print("Distance 1 / 2 [cm]: ");
          Serial.println(maxRangeMM/10); // cm
        }
      }
      
      COUNTBACK = COUNTBACK - INFO_WAIT_TIME/1000;
      infotimer = millis();      

    }

    // readSerial(Serial1);
    // readSerial(Serial2);


  if (getSerial1Data == true) {

    noSerial1Data = 0;
    
    if (distanceFromSerial1 > 0) {
       distanceCM1 = distanceFromSerial1/10;
    } else {
      Serial.print("s1  Wrong distance result: ");
      Serial.println(distanceFromSerial1);
      // longtimer -= LONG_WAIT_TIME*0.8;
    }
    
    getData1timer = millis();
    getSerial1Data = false;
  }

  if (getSerial2Data == true) {

    noSerial2Data = 0;

    if (distanceFromSerial2 > 0) {
       distanceCM2 = distanceFromSerial2/10;
    } else {
      Serial.print("s2  Wrong distance result: ");
      Serial.println(distanceFromSerial2);
      // longtimer -= LONG_WAIT_TIME*0.8;
    }
    getData2timer = millis();
    getSerial2Data = false;
  }

  
  if (distanceCM1>0 || distanceCM2>0) 
  {
    // delay(longWaitingTime);
    count=0;
  }
  else
  {
    count++;
    delay(100);
    if (count>=10) //to offen no data
    {
      count=0;   
    }
  }

  distanceProc1 = (float)distanceCM1/(maxRangeMM/10)*100;
  distanceProc2 = (float)distanceCM2/(maxRangeMM/10)*100;
  
  bar_y1 = round((float)distanceProc1/100*(barEndY-barStartY));
  bar_y2 = round((float)distanceProc2/100*(barEndY-barStartY));
  
  display_data();
  oled.sendBuffer();

  // Wait 2 seconds and do next measurement
  //delay(500);

}

/* **********************************************************
/*                 Functions and Subroutines                *
/* ******************************************************** */

void display_data() {
    oled.setCursor(70, 38);
    oled.println(COUNTBACK);
  
    char cmStr[12];
    char cmStrFull[24];

    int pos1 = -2;
    int pos2 = -2;
    
    sprintf(cmStr, "%f", distanceCM1);
    sprintf(cmStrFull, "%i cm", (int)distanceCM1);

    oled.setCursor(10, 15);
    oled.println("Sensor 1:");
    oled.setCursor(30, 28);
    // oled.println(cm, DEC);
    oled.println(cmStrFull);
    
    // oled.drawFrame(78, 0, sizeX-78, sizeY);
    oled.drawFrame(barStartX, barStartY, 2*barWidth, barEndY-barStartY);
    // oled.drawBox(0, 32, round((float)distanceProc1/100*sizeX), 2);
    // if (distanceCM1==0)
    if (distanceFromSerial1==-1)
    {
      // oled.drawLine(78, 0, 103, sizeY);
      // oled.drawLine(103, 0, 78, sizeY);
      oled.drawLine(barStartX, 0, barStartX+barWidth, barEndY-barStartY);
      oled.drawLine(barStartX+barWidth, 0, barStartX, barEndY-barStartY);
    }
    else
    {
      if (distanceFromSerial1>0 && noSerial1Data < 5)
        {
          // oled.drawBox(78, round((float)distanceProc1/100*sizeY), 25, sizeY-round((float)distanceProc1/100*sizeY));
          oled.drawBox(barStartX, bar_y1, barWidth, (barEndY-barStartY)-bar_y1);
        }
        else
        {

            if (maxRangeMM/2/10>distanceCM1) {
              pos1 = 10;
            }   
            
          oled.drawHLine(barStartX, bar_y1, barWidth);
          
          oled.setCursor(barStartX + 2, bar_y1 + pos1);
          if (noSerial1Data >= 5) {
              oled.println(noSerial1Data);            
            } else {
              oled.println(distanceFromSerial1);          
            }     
        }
    }
      
    char lcmStr[12];
    char lcmStrFull[24];
    sprintf(lcmStr, "%f", distanceCM2);
    sprintf(lcmStrFull, "%i cm", (int)distanceCM2);

    oled.setCursor(10, 47);
    oled.println("Sensor 2:");
    oled.setCursor(30, 60);
    // oled.println(cm, DEC);
    oled.println(lcmStrFull);
    
    // oled.drawBox(0, 62, round((float)distanceProc2/100*sizeX), 2);
    //if (distanceCM2==0)
    if (distanceFromSerial2==-1)
    {
      // oled.drawLine(104, 0, sizeX, sizeY);
      // oled.drawLine(sizeX, 0, 104, sizeY);
      oled.drawLine(barStartX+barWidth, 0, barStartX+barWidth+barWidth, barEndY-barStartY);
      oled.drawLine(barStartX+barWidth+barWidth, 0, barStartX+barWidth, barEndY-barStartY);
    }
    else
    {
      // oled.drawBox(104, round((float)distanceProc2/100*sizeY), 25, sizeY-round((float)distanceProc2/100*sizeY));
      // oled.drawBox(barStartX+barWidth, round((float)distanceProc2/100*(barEndY-barStartY)), barWidth, (barEndY-barStartY)-round((float)distanceProc2/100*(barEndY-barStartY)));
       if (distanceFromSerial2>0 && noSerial2Data < 5)
          {
            // oled.drawBox(104, round((float)distanceProc2/100*sizeY), 25, sizeY-round((float)distanceProc2/100*sizeY));
            oled.drawBox(barStartX+barWidth, bar_y2, barWidth, (barEndY-barStartY)-bar_y2);            
          }
          else
          {
            // oled.drawLine(barStartX+barWidth, round((float)distanceProc1/100*(barEndY-barStartY)),barWidth, 0);
            oled.drawHLine(barStartX+barWidth, bar_y2, barWidth);

            if (maxRangeMM/2/10>distanceCM2) {
              pos2 = 10;
            }              
            
            oled.setCursor(barStartX+barWidth + 2, bar_y2 + pos2);
            if (noSerial2Data >= 5) {
              oled.println(noSerial2Data);            
            } else {
              oled.println(distanceFromSerial2);          
            }            
          }
    }    
}


void triggerGetDataToSerial(HardwareSerial localSerial, String info){
  // https://wolles-elektronikkiste.de/hc-sr04-und-jsn-sr04t-2-0-abstandssensoren
  unsigned int distance;
  bool DEBUG = true;
  
  // start trigger on serial
  if (DEBUG)
  {    
    Serial.println("=========================================================");
    Serial.print(info);
    Serial.println(" DEBUG triggerGetDataToSerial --> ");    
  }

  localSerial.write(0x55);
  // delay(50);

}

void serialEvent1() {
  distanceFromSerial1 = serialEventResult(Serial1, "s1 ");
  getSerial1Data = true;
}

void serialEvent2() {
  distanceFromSerial2 = serialEventResult(Serial2, "s2 ");
  getSerial2Data = true;
}

int serialEventResult(HardwareSerial localSerial, String info) {
  bool DEBUG = true;

  unsigned int distance;
  
  byte startByte, h_data, l_data, sum = 0;
  byte buf[3];

  while (localSerial.available()) {
  
    startByte = (byte)localSerial.read();
    
    if(startByte == 255){
      localSerial.readBytes(buf, 3);
      h_data = buf[0];
      l_data = buf[1];
      sum = buf[2];
      distance = (h_data<<8) + l_data;
      
      if(((startByte + h_data + l_data)&0xFF) != sum){
        if (DEBUG)
          {
            Serial.print(info);
            Serial.println(" DEBUG Invalid checksum"); 
            Serial.print("Received Data => ");

            for(i=0; i<sizeof(buf); i++){
               printHex(buf[i]);
            }
            Serial.println("");
          }
        //return 0;
        // distanceFromSerial1 = -3;
        return -3;
      }
      else
      {
        if (DEBUG)
          {
            Serial.print(info);
            Serial.print(" DEBUG Distance [mm]: "); 
            Serial.println(distance);            
          }
          if (distance<maxRangeMM) // mm
          {
            //return distance;
            // distanceFromSerial1 = distance;
            return distance;
          }
          else
          {
            //return 0;
            // distanceFromSerial1 = -9;
            return -9;
          }
                  
      } 
    } 
    else
    {
      if (DEBUG)
      {
        Serial.print(info);
        Serial.print(" DEBUG Invalid startByte: "); 
        // Serial.println(startByte );
        Serial.println(convertToHex(startByte));
        // localSerial.readBytes(buf, 3);
        // Serial.print("Received Data => ");
        // Serial.print(buf[0]);
        // Serial.print(buf[1]);
        // Serial.println(buf[2]);
        // Serial.println(buf)
        // for(i=0; i<sizeof(buf); i++){
        //    printHex(buf[i]);
        // }
        Serial.println("");
      }
      // return 0;
      // distanceFromSerial1 = -2;
      return -2;
    }
  }
}

void readSerial(HardwareSerial localSerial){  
  bool DEBUG = true;
    
  while(localSerial.available()>0){
    String s1 = localSerial.readString();// s1 is String type variable.
    Serial.print("Received Data => ");
    Serial.println(s1);//display same received Data back in serial monitor.
  }

}

void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
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
