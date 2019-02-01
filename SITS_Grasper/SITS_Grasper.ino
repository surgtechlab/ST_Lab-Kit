/* 
 *  This program has been written by James Kinch on behalf of the Surgical Technologies Laboratory
 *  at the University of Leeds.
 *  
 *  This code is written to work with the Adafruit Huzzah EPS8266 Featherboard.
 *  
 *  Feel free to edit as required. 
 */

// Screen Setup
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <stdint.h>

Adafruit_PCD8544 display = Adafruit_PCD8544(14, 13, 12, 2, 16); //CLK, DIN, D/C, SCE, RST - LED is 15

#define NUMFLAKES 10;
#define XPOS 0;
#define YPOS 1;
#define DELTAY 2;

#define LOGO16_GLCD_HEIGHT 16;
#define LOGO16_GLCD_WIDTH  16;

//Creating the vars for force bar widths - scale this if adding channels
int16_t comp_w = 0; //compressive force bar width
int16_t shr_w = 0; //shear force bar width
int16_t comp_x = 42; //compressive force bar x start position on screen
int16_t shr_x = 42; //shear foce bar x start position on screen

int comp_force; //variable for compressive force calculation
int shr_force; //variable for shear force calculation


//Pressure Sensor Setup
int presPin = 34; //set pressure sensor analog pin input
int presVal = 0;

//Setting up Inductive Sensor

uint32_t t_start = 0;
unsigned long CH0_Offset = 0; //channel 0 offset variable
unsigned long CH1_Offset = 0; //channel 1 offset variable

int LDC = 0x2A; // 0x2A=ADR Low, 0x2B=ADR High

//reg for channels 0 and 1
  //channel 0 most significant and least significant bit
int CH0MSB = 0x00;
int CH0LSB = 0x01;
  //channel 1 most significant and least significant bit
int CH1MSB = 0x02;
int CH1LSB = 0x03;

extern unsigned char SmallFont[];
//****************************************8

//read channel 0 function - this is called later in the code
unsigned long readChannel0()
{
  unsigned long val = 0;
  word c = 0;
  word d = 0;
  c = readValue(LDC, CH0MSB);
  d = readValue(LDC, CH0LSB);
  val = c;
  val <<= 16;
  val += d;
  return val;
}

//read channel 1 function - this is called later in the code
unsigned long readChannel1()
{
  unsigned long val1 = 0;
  word c1 = 0;
  word d1 = 0;
  c1 = readValue(LDC, CH1MSB);
  d1 = readValue(LDC, CH1LSB);
  val1 = c1;
  val1 <<= 16;
  val1 += d1;
  return val1;
}

//generic read funciton for I2C communications - is called within the individual channel read functions above. 
word readValue (int LDC, int reg)
{
  int a = 0;
  int b = 0;
  word value = 0;
  Wire.beginTransmission(LDC);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(LDC, 2);
  
  while (Wire.available())
  {
    a = Wire.read();
    b = Wire.read();
  }
 
  value = a;
  value <<= 8;
  value += b;
  return value;
}

//function used to write the configuration of the I2C bus  
void writeConfig(int LDC, int reg, int MSB, int LSB)
{
  Wire.beginTransmission(LDC);
  Wire.write(reg);
  Wire.write(MSB);
  Wire.write(LSB);
  Wire.endTransmission();
}

 

void Configuration()
{
  int ta = 0;
  int tb = 0;
  
  Serial.println("\n Configuring LDC1614....");

// single channel - DO NO USE THIS IF REQUIRING MULTIPLE READINGS
//  writeConfig(LDC, 0x1C, 0x80, 0x00); //RESET_DEV (all registers back to default)
//  writeConfig(LDC, 0x1A, 0x36, 0x81); //Enter Sleep Mode - to CONFIG (Channel_mode=single and clock source=ext) P11 and P31 datasheet
//  writeConfig(LDC, 0x1B, 0x02, 0x0D); //Configure channel mode = single, channel select (0-1) and Filter Bandwidth = 10MHz - THIS NEEDS BE CHANGED TO MULTIPLE
//  writeConfig(LDC, 0x14, 0x10, 0x02); //CLOCK_DIVIDERS_CHx (Frequency divider registers CH0 = 0x14 Datasheet Ref: P10 and P27
//  writeConfig(LDC, 0x1E, 0xF0, 0x00); //DRIVE_CURRENT_CHx (Datasheet Ref: P14 and P33)
//  writeConfig(LDC, 0x10, 0x00, 0x0A); //CHx_SETTLECOUNT (Datasheet Ref: P13 and P25)
//  writeConfig(LDC, 0x08, 0x04, 0x00); //CHx_RCOUNT (Datasheet Ref: P13 and P22)
//  writeConfig(LDC, 0x0C, 0x00, 0x00); //OFFEST_CHx (Datasheet Ref: P14 and P24)
//  writeConfig(LDC, 0x1A, 0x00, 0x81); //Exit Sleep mode (CHANGE THIS?!) was 0x1A 0x0e 0x81

//Multiple channel
  writeConfig(LDC, 0x1C, 0x80, 0x00); //RESET_DEV (all registers back to default)
  writeConfig(LDC, 0x1A, 0x36, 0x81); //Enter Sleep Mode - to CONFIG (Channel_mode=single and clock source=ext) P11 and P31 datasheet
  writeConfig(LDC, 0x1B, 0x82, 0x0D); //Configure channel mode = single, channel select (0-1) and Filter Bandwidth = 10MHz

  //SCALE ALL OF THE FOLLOWING TO ADD CHANNEL READS TO THE PROGRAM - NEW VARIABLES AND READ FUNCTIONS WILL ALSO NEED TO BE ADDED
  writeConfig(LDC, 0x14, 0x10, 0x02); //CLOCK_DIVIDERS_CHx (Frequency divider registers CH0 = 0x14 Datasheet Ref: P10 and P27
  writeConfig(LDC, 0x15, 0x10, 0x02); //CLOCK_DIVIDERS_CHx (Frequency divider registers CH1 = 0x15 Datasheet Ref: P10 and P27
  
  writeConfig(LDC, 0x1E, 0xF0, 0x00); //DRIVE_CURRENT_CHx (Datasheet Ref: P14 and P33) CH0
  writeConfig(LDC, 0x1F, 0xF0, 0x00); //DRIVE_CURRENT_CHx (Datasheet Ref: P14 and P33) CH1
  
  writeConfig(LDC, 0x10, 0x00, 0x0A); //CHx_SETTLECOUNT (Datasheet Ref: P13 and P25) CH0
  writeConfig(LDC, 0x11, 0x00, 0x0A); //CHx_SETTLECOUNT (Datasheet Ref: P13 and P25) CH1
  
  writeConfig(LDC, 0x08, 0x04, 0x00); //CHx_RCOUNT (Datasheet Ref: P13 and P22) CH0
  writeConfig(LDC, 0x09, 0x04, 0x00); //CHx_RCOUNT (Datasheet Ref: P13 and P22) CH1
  
  writeConfig(LDC, 0x0C, 0x00, 0x00); //OFFEST_CHx (Datasheet Ref: P14 and P24) CH0
  writeConfig(LDC, 0x0D, 0x00, 0x00); //OFFEST_CHx (Datasheet Ref: P14 and P24) CH1

  
  //this does not need to be scaled
  writeConfig(LDC, 0x1A, 0x00, 0x81); //Exit Sleep mode (CHANGE THIS?!) was 0x1A 0x0e 0x81

  //debugging purposes
  ta = readValue(LDC, 0x7E);
  Serial.println("\n DEBUG - config:manu");
  Serial.println(ta);
  Serial.println(" System Configured \n");
  delay(500);
  
}
 

void setup() {
  //Inductive Sensor Setup
  Wire.begin();

  Serial.begin(9600);

  Serial.println("\n SITS Test Program v2");
  Serial.println("********************** \n");
  
  Configuration();

  delay(500);

  t_start = millis(); //get global start time


 //defining channel offset variables - scale these
  CH0_Offset = readChannel0(); //Get bias reading
  Serial.print("Bias Reading: ");
  Serial.println(CH0_Offset);
   
  CH1_Offset = readChannel1(); //Get bias reading
  Serial.print("Bias Reading: ");
  Serial.println(CH1_Offset);
  

  //display init
  display.begin();
  display.setContrast(65);
  pinMode(15, OUTPUT); //LED backlight power
  digitalWrite(15,HIGH); //required to use dacWrite as analogWrite not a completed fuction in Esspresif lib.

  display.clearDisplay();   // clears the screen and buffer

//  display.fillRect(10, 20, 20, 10, BLACK);
//  display.display();
//  delay(2000);
//  display.clearDisplay();

  //displaying opening screen
  display.setTextSize(1.5);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("University of Leeds");
  display.setCursor(0,20);
  display.println("Surg Tech Lab");
  display.display();
  delay(2000);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Soft");
  display.setCursor(0,10);
  display.println("Inductive");
  display.setCursor(0,20);
  display.println("Tactile Sensor");
  display.display();
  delay(4000);


}

void loop() 
{  
  
//Running data acquisition 
  //CH0
signed long calibrated_data0 = 0;
unsigned long CH0_Gain = 0.00001;
unsigned long data0 = 0;
 //CH1
signed long calibrated_data1 = 0;
unsigned long CH1_Gain = 0.00001;
unsigned long data1 = 0;

uint32_t t_current = millis();

    for (int i=0; i<84; i++)
    {
      //getting data from i2c chips
      //these need to be scaled for additional coils
      unsigned long data0 = readChannel0();
      calibrated_data0 = data0-CH0_Offset;
      calibrated_data0 = calibrated_data0 * 0.00001;

      unsigned long data1 = readChannel1();
      calibrated_data1 = data1-CH1_Offset;
      calibrated_data1 = calibrated_data1 * 0.00001;
      
      Serial.println(calibrated_data0); //for debugging
      Serial.println(calibrated_data1); //for debugging

      //displaying data
      display.clearDisplay();   // clears the screen and buffer
      display.setTextSize(1.5);
      display.setTextColor(BLACK);

      //Displaying the compressive force bar
      display.setCursor(0,0);
      display.println("Comp. Force: ");
      comp_force = (calibrated_data0+calibrated_data1)*3;
        //making sure there is no 0 value
      if (comp_force == 0) {
        comp_w = 1;
        comp_x = 41;
      }
      else if (comp_force < 0)  {
        //42
        comp_x = 42 + comp_force;
        comp_w = 42 - comp_x;
      }
      else {
        comp_x = 42;
        comp_w = comp_force;
      }
      display.fillRect(comp_x, 10, comp_w, 9, BLACK); //creating the bar

//      //Displaying the shear force bar
      display.setCursor(0,20);
      display.println("Shr. Force: ");
      shr_force = calibrated_data0-calibrated_data1;
        //making sure there is no 0 value
      if (shr_force == 0) {
        shr_w = 1;
        shr_x = 41;
      }
      else if (shr_force < 0)  {
        //42
        shr_x = 42 + shr_force;
        shr_w = 42 - shr_x;
      }
      else {
        shr_x = 42;
        shr_w = shr_force;
      }
      display.fillRect(shr_x, 30, shr_w, 9, BLACK); //creating the bar
      
      display.display();

      delay(20);
    }


}
