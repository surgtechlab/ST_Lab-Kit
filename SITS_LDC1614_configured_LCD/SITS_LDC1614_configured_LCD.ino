// LDC1614EVM 3.3 v to Arduino 3.3v
// LDC1614EVM GND to Arduino GND
// LDC1614EVM INTB to Arduino GND
// LDC1614EVM SD to Arduino GND
// LDC1614EVM ADDR to Arduino GND (works better than at 3.3 v)
// LDC1614EVM SDA to Arduino SDA ( 4.7K pull up resistors)
// LDC1614EVM SCL to Arduino SCL (4.7K pull up resistors)


#include <Wire.h>
#include <LCD5110_Graph.h>

uint32_t t_start = 0;
unsigned long CH0_Offset = 0;

int LDC = 0x2A; // 0x2A=ADR Low, 0x2B=ADR High

int CH0MSB = 0x00;
int CH0LSB = 0x01;1
LCD5110 myGLCD(SCK,MOSI,8,6,10);
extern unsigned char SmallFont[];
//****************************************8


unsigned long readChannel0()

{

  unsigned long val = 0;

  word c = 0;

  word d = 0;

  c = readValue(LDC, CH0MSB);

  d = readValue(LDC, CH0LSB);
//  Serial.println("\n DEBUG - readchannel c and d");
//  Serial.println(c);
//  Serial.println(d);

  val = c;

  val <<= 16;

  val += d;

  return val;

}

 
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

//  Serial.println("\n DEBUG - readvalue a and b");
//  Serial.println(a);
//  Serial.println(b);
  
  value = a;

  value <<= 8;

  value += b;

  return value;

}

 

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
  

//  writeConfig(LDC, 0x14, 0x10, 0x02);//CLOCK_DIVIDERS_CH0
//
//  writeConfig(LDC, 0x1E, 0x90, 0x00);//DRIVE_CURRENT_CH0
//
//  writeConfig(LDC, 0x10, 0x00, 0x0A);//SETTLECOUNT_CH0
//
//  writeConfig(LDC, 0x08, 0x04, 0xD6);//RCOUNT_CH0
//
//  writeConfig(LDC, 0x15, 0x10, 0x02);//CLOCK_DIVIDERS_CH1
//
//  writeConfig(LDC, 0x1F, 0x90, 0x00);//DRIVE_CURRENT_CH1
//
//  writeConfig(LDC, 0x11, 0x00, 0x0A);//SETTLECOUNT_CH1
//
//  writeConfig(LDC, 0x09, 0x04, 0xD6);//RCOUNT_CH1
//
//  writeConfig(LDC, 0x19, 0x00, 0x00);//ERROR_CONFIG
//
//  writeConfig(LDC, 0x1B, 0x82, 0x0C);//MUX_CONFIG

writeConfig(LDC, 0x1C, 0x80, 0x00); //RESET_DEV (all registers back to default)
writeConfig(LDC, 0x1A, 0x36, 0x81); //Enter Sleep Mode - to CONFIG (Channel_mode=single and clock source=ext) P11 and P31 datasheet
writeConfig(LDC, 0x1B, 0x02, 0x0D); //Configure channel mode = single, channel select (0-1) and Filter Bandwidth = 10MHz
writeConfig(LDC, 0x14, 0x10, 0x02); //CLOCK_DIVIDERS_CHx (Frequency divider registers CH0 = 0x14 Datasheet Ref: P10 and P27
writeConfig(LDC, 0x1E, 0xF0, 0x00); //DRIVE_CURRENT_CHx (Datasheet Ref: P14 and P33)
writeConfig(LDC, 0x10, 0x00, 0x0A); //CHx_SETTLECOUNT (Datasheet Ref: P13 and P25)
writeConfig(LDC, 0x08, 0x04, 0x00); //CHx_RCOUNT (Datasheet Ref: P13 and P22)
writeConfig(LDC, 0x0C, 0x00, 0x00); //OFFEST_CHx (Datasheet Ref: P14 and P24)
writeConfig(LDC, 0x1A, 0x00, 0x81); //Exit Sleep mode (CHANGE THIS?!) was 0x1A 0x0e 0x81

  ta = readValue(LDC, 0x7E);

  //d = readValue(LDC, CH0LSB);
Serial.println("\n DEBUG - config:manu");
Serial.println(ta);
//  Serial.println(d);



  Serial.println(" System Configured \n");
  delay(500);
  
}
 

void setup()

{

  Wire.begin();

  Serial.begin(9600);

  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

  Serial.println("\n SITS Test Program v2");
  Serial.println("********************** \n");
  
  Configuration();

  delay(500);

  t_start = millis(); //get global start time
  
  CH0_Offset = readChannel0(); //Get bias reading
  Serial.print("Bias Reading: ");
  Serial.println(CH0_Offset);
}

 

void loop()

{

signed long calibrated_data0 = 0;
unsigned long CH0_Gain = 0.00001;
uint32_t t_current = millis();
unsigned long data0 = 0;
int y=0;

myGLCD.clrScr();
myGLCD.drawRect(0, 0, 83, 47);
myGLCD.drawLine(0, 23, 84, 23);
myGLCD.drawLine(41, 0, 41, 47);
myGLCD.print("SITS XYZ Demo", CENTER, 0);
myGLCD.update();

//Serial.println(t_current - t_start);
//Serial.println(" ");

// Graph display

    for (int i=0; i<84; i++)
    {
      unsigned long data0 = readChannel0();
      calibrated_data0 = data0-CH0_Offset;
      calibrated_data0 = calibrated_data0 * 0.00001;
      
      Serial.println(calibrated_data0);

      y= 46-calibrated_data0;
      myGLCD.invPixel(i, y);
      myGLCD.update();
      delay(20);
    }


}
