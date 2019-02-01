/* 
 *  This program has been written by Pete Culmer, Surgical Technologies Laboratory,
 *  University of Leeds, 2018
 *  
 *  This code is written to work with the Arduino Due
 *  
 *  Provides XYZ sensing of the SITS 4 coil sensor

 PIN OUTS
**********************************************************************
 LDC1614EVM 3.3 v to Arduino 3.3v
 LDC1614EVM GND to Arduino GND
 LDC1614EVM INTB to Arduino GND
 LDC1614EVM SD to Arduino GND
 LDC1614EVM ADDR to Arduino GND (works better than at 3.3 v)
 LDC1614EVM SDA to Arduino SDA ( 4.7K pull up resistors)
 LDC1614EVM SCL to Arduino SCL (4.7K pull up resistors)
 *********************************************************************
 */

#include <Wire.h>
#include <LCD5110_Graph.h>


// LDC Configuration
// **********************************************************
// Define LDC Device I2C address 
int LDC = 0x2A; // 0x2A=ADR Low, 0x2B=ADR High

//DEfine output registers for channels 0-3
  //channel 0 most significant and least significant bit
int CH0MSB = 0x00;
int CH0LSB = 0x01;
  //channel 1 most significant and least significant bit
int CH1MSB = 0x02;
int CH1LSB = 0x03;
  //channel 2 most significant and least significant bit
int CH2MSB = 0x04;
int CH2LSB = 0x05;
  //channel 3 most significant and least significant bit
int CH3MSB = 0x06;
int CH3LSB = 0x07;


// Measurement Calibration
unsigned long CH0_Offset = 0; //channel 0 offset variable
unsigned long CH1_Offset = 0; //channel 1 offset variable
unsigned long CH2_Offset = 0; //channel 0 offset variable
unsigned long CH3_Offset = 0; //channel 1 offset variable
unsigned long CH_Gain = 1; //Same gain applied to all coils


//*************************************************************

//TIMING VARS
unsigned long startmicros = 0L;
unsigned long currentmicros = 0L;
unsigned long period = 0L;


LCD5110 myGLCD(SCK,MOSI,8,6,10);
extern unsigned char SmallFont[];
//****************************************8

//Generic read channel function - supercedes invidual channel read functions
unsigned long readChannel(int ChMSB,int ChLSB,unsigned long gain, unsigned long offset)
{
  unsigned long val = 0;
  signed long calib = 0;
  word c = 0;
  word d = 0;
  c = readValue(LDC, ChMSB);
  d = readValue(LDC, ChLSB);
  val = c;
  val <<= 16;
  val += d;
  calib = (val - offset);
  return calib;
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

  //Prepare Device
  // writeConfig(int LDC, int reg, int MSB, int LSB)
  writeConfig(LDC, 0x1C, 0x80, 0x00); //RESET_DEV (all registers back to default)
  writeConfig(LDC, 0x1A, 0x36, 0x81); //Enter Sleep Mode - to CONFIG (Channel_mode=single and clock source=ext) P11 and P31 datasheet

  //Configure channel mode = Multi, channel select (0,1,2,3) and Filter Bandwidth = 10MHz
  writeConfig(LDC, 0x1B, 0xC2, 0x0D); 

  //Define Frequency Dividers (Frequency divider registers CH0-3 = 0x14-17 Datasheet Ref: P10 and P27
  writeConfig(LDC, 0x14, 0x10, 0x02); //CLOCK_DIVIDERS_CHx 
  writeConfig(LDC, 0x15, 0x10, 0x02); //CLOCK_DIVIDERS_CHx 
  writeConfig(LDC, 0x16, 0x10, 0x02); //CLOCK_DIVIDERS_CHx 
  writeConfig(LDC, 0x17, 0x10, 0x02); //CLOCK_DIVIDERS_CHx 

  //DRIVE_CURRENT_CHx (Datasheet Ref: P14 and P33) CH0-Ch3
  writeConfig(LDC, 0x1E, 0xF0, 0x00); 
  writeConfig(LDC, 0x1F, 0xF0, 0x00);
  writeConfig(LDC, 0x20, 0xF0, 0x00); 
  writeConfig(LDC, 0x21, 0xF0, 0x00); 

  //CHx_SETTLECOUNT (Datasheet Ref: P13 and P25) CH0-3
  writeConfig(LDC, 0x10, 0x00, 0x0A); 
  writeConfig(LDC, 0x11, 0x00, 0x0A); 
  writeConfig(LDC, 0x12, 0x00, 0x0A); 
  writeConfig(LDC, 0x13, 0x00, 0x0A); 

  //CHx_RCOUNT (Datasheet Ref: P13 and P22) CH0-3
  writeConfig(LDC, 0x08, 0x04, 0x00); 
  writeConfig(LDC, 0x09, 0x04, 0x00); 
  writeConfig(LDC, 0x0A, 0x04, 0x00); 
  writeConfig(LDC, 0x0B, 0x04, 0x00);
    
  //Freq Offset_CHx (Datasheet Ref: P14 and P24) CH0-3
  writeConfig(LDC, 0x0C, 0x00, 0x00); 
  writeConfig(LDC, 0x0D, 0x00, 0x00);
  writeConfig(LDC, 0x0E, 0x00, 0x00);
  writeConfig(LDC, 0x0F, 0x00, 0x00);

  Serial.print("\n Config Done - Now Exit Sleep");
  delay(1000);
 
  //Exit Sleep mode (CHANGE THIS?!) was 0x1A 0x0e 0x81 //  writeConfig(LDC, 0x1A, 0x00, 0x81); 
//  writeConfig(LDC, 0x1A, 0x0e, 0x81); 
  writeConfig(LDC, 0x1A, 0x00, 0x81); 

  //debugging purposes
  ta = readValue(LDC, 0x7F);
  Serial.print("\n DEBUG - config:manu ");
  Serial.println(ta);
  Serial.print(" System Config:");
  ta = readValue(LDC, 0x1A);
  Serial.println(ta);
  delay(500);
  
}
 

void setup()

{

  Wire.begin();

  Serial.begin(115200);

  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

  Serial.print("\n SITS XYZ Test");
  Serial.println("********************** \n");
  
  Configuration();

  delay(500);

  //REMOVE STATIC OFFSETS FROM COIL READINGS
  
  //defining channel offset variables - scale these
  CH0_Offset = readChannel(CH0MSB,CH0LSB,1,0);
  Serial.print("Bias Reading: ");
  Serial.println(CH0_Offset);
  
  CH1_Offset = readChannel(CH1MSB,CH1LSB,1,0);
  Serial.print("Bias Reading: ");
  Serial.println(CH1_Offset);

  CH2_Offset = readChannel(CH2MSB,CH2LSB,1,0);
  Serial.print("Bias Reading: ");
  Serial.println(CH2_Offset);

  CH3_Offset = readChannel(CH3MSB,CH3LSB,1,0);
  Serial.print("Bias Reading: ");
  Serial.println(CH3_Offset);
}

void loop()
{

//DEFINE VARIABLES
//register reads
unsigned long ch0_read = 0, ch1_read = 0, ch2_read = 0, ch3_read = 0;
long load_X = 0,load_Y = 0,load_Z = 0;
int y=0;
int screenY_offset = 46;
int err = 0;

//Reset LCD Screen ********
myGLCD.clrScr();
myGLCD.drawRect(0, 0, 83, 47);
myGLCD.drawLine(0, 23, 84, 23);
myGLCD.drawLine(41, 0, 41, 47);
myGLCD.print("SITS XYZ-v2", CENTER, 0);
myGLCD.update();
//***********************

// Graph display

    //PLOT ACROSS PIXELS OF THE LCD SCREEN
    for (int i=0; i<84; i++)
    {
      //READ CH0-3
      startmicros = micros (); //get start time of read
      ch0_read = readChannel(CH0MSB,CH0LSB,CH_Gain,CH0_Offset);
      ch1_read = readChannel(CH1MSB,CH1LSB,CH_Gain,CH1_Offset);
      ch2_read = readChannel(CH2MSB,CH2LSB,CH_Gain,CH2_Offset);
      ch3_read = readChannel(CH3MSB,CH3LSB,CH_Gain,CH3_Offset);
      currentmicros = micros ();
      period = currentmicros - startmicros;
      
      //Define linearised outputs for X,Y,Z measures
      load_X = -ch0_read - ch1_read + ch2_read + ch3_read;
      load_Y = -ch0_read + ch1_read + ch2_read - ch3_read;
      load_Z = ch0_read + ch1_read + ch2_read + ch3_read;

      Serial.print(ch0_read);
      Serial.print(" ");
      Serial.print(ch1_read);
      Serial.print(" ");
      Serial.print(ch2_read);
      Serial.print(" ");
      Serial.print(ch3_read);
      Serial.print("\n");

        //err = readValue(LDC, 0x1A);
        //Serial.print("DEBUG - Err:0x18: ");
        //Serial.println(err);
        
//      Serial.print(load_X/1000);
//      Serial.print(" ");
//      Serial.print(load_Y/1000);
//      Serial.print(" ");
//      Serial.println(load_Z/1000);

      //Serial.print(" ");
      //Serial.println; //SWITCH THIS TO PRINT TIMING INFO
      //Serial.println(period);   
      
      y= screenY_offset - (load_X/50000);
      myGLCD.invPixel(i, y);
      y= screenY_offset - (load_Y/50000);
      myGLCD.invPixel(i, y);
      y= screenY_offset - (load_Z/50000);
      myGLCD.invPixel(i, y);      
      myGLCD.update();
      delay(200);
    }


}
