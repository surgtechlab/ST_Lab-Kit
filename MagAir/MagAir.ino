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
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

// Software SPI (slower updates, more flexible pin options):
// pin 5 - Serial clock out (SCLK)
// pin 18 - Serial data out (DIN)
// pin 33 - Data/Command select (D/C)
// pin 12 - LCD chip select (CS)
// pin 27 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 18, 33, 12, 27);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

//Pressure Sensor Setup
int presPin = 34; //set pressure sensor analog pin input
int presVolt = 0;
int presVal = 0;

//Setting up MagOne
#include<Wire.h>
#define Addr 0x0C
#include<stdint.h>

void setup() {
  Serial.begin(9600); //  setup serial

  //display init
  display.begin();
  display.setContrast(65);
  pinMode(15, OUTPUT); //LED backlight power
  digitalWrite(15,HIGH); //required to use dacWrite as analogWrite not a completed fuction in Esspresif lib.

  display.clearDisplay();   // clears the screen and buffer

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
  display.println("Soft Tactile");
  display.setCursor(0,10);
  display.println("Sensor with");
  display.setCursor(0,20);
  display.println("Variable");
  display.setCursor(0,30);
  display.println("Compliance");
  display.display();
  delay(4000);

  //I2C MagOne Setup
  Wire.begin();  // Initialise I2C ]
  Serial.begin(9600);// Initialise serial communication, Baud rate = 9600
  Wire.beginTransmission(Addr); // Start I2C Transmission
  /*********************************************************************************** 
                                EXIT AND RESET MODE
   ***********************************************************************************/
  Wire.write(0x80); // Exit Mode
  Wire.write(0xF0); // Reset Command
  Wire.endTransmission();  // Stop I2C 
  Serial.print("\n Reset\n");
  delay(1000); 
  Serial.print("\n Setup Starting...\n");
   /*********************************************************************************** 
                                  SELECT GAIN
   ***********************************************************************************/
  Wire.beginTransmission(Addr); // Start I2C Transmission
  Wire.write(0x60); // Write register command 
  Wire.write(0x00); // Set AH = 0x00, BIST disabled           CMD b#2 D[15:8] 0000 0000
  Wire.write(0x0C); // Set AL = 0x0C, GAIN = 0                CMD b#3 D[7:0]  0000 1100
  Wire.write(0x00); // Select address register, (0x00 << 2)   CMD b#2 A[5:0] << 2
  // Z_series = b0, Gain_Sel = b000, Hall_Conf = b1100 (All at default settings)
  Wire.endTransmission();  // Stop I2C 
    // Stop I2C  
  Wire.requestFrom(Addr, 1); // Request 1 byte of data
  Serial.print(Wire.available());
  if(Wire.available() == 1) // Read status byte
  {
   uint8_t ca = Wire.read();
   Serial.print("\n Register 0 Status: ");
   Serial.print(ca,BIN);
   Serial.print("\n");
  }
  /*********************************************************************************** 
                               SELECT BURST MODE
   ***********************************************************************************/
  Wire.beginTransmission(Addr);
  Wire.write(0x60); // Write register command  
  Wire.write(0x03);// Write commands for Burst Mode         CMD b#2 D[15:8] 0000 0011
  Wire.write(0x80); //                                      CMD b#3 D[7:0]  1000 0000                                    // CMD b#3 D[7:0]  1000 0000
  Wire.write(0x04);// Select address reg 1 (<<2)            
  // Trig_Int_Sel =b1, Burst data rate = b00 0011
  Wire.endTransmission();// Stop I2C Transmission
  
  Wire.requestFrom(Addr, 1);   // Request 1 byte of data
  if(Wire.available() == 1)   // status byte
  {
   uint8_t cb = Wire.read();
   Serial.print("\n Register 1 Status: ");
   Serial.print(cb,BIN);
   Serial.print("\n");
   }
  delay(100);
  /*********************************************************************************** 
                               SELECT RESOLUTION
   ***********************************************************************************/
  Wire.beginTransmission(Addr);  
  Wire.write(0x60); // READ register command  
  Wire.write(0x02);
  Wire.write(0x04);
  Wire.write(0x08);// Select address reg 0 (<<2)            
  Wire.endTransmission(); // Stop I2C 
  
  Wire.requestFrom(Addr, 3);  // Request 3 bytes of data
  if(Wire.available() == 3)  // Read status byte
  {
     delay(5);
     uint8_t c = Wire.read();
     uint8_t ra = Wire.read();
     uint8_t rb = Wire.read();
     Serial.print("\n Register Read Status: ");
     Serial.print(c,BIN);
     Serial.print("\t Reg-MSB: ");
     Serial.print(ra,BIN);
     Serial.print("\t Reg-LSB: ");
     Serial.print(rb,BIN);
     Serial.print("\n");
     // Register Config   (0) 1011 0100  0001 1100    hallconf = 1100 gainsel=001 zseries=0
     // Register Config   (0) 0000 0000  0000 1100
     // Register Config  (4) 1011 0100 0001 0100
     // Register Config  (8) 1011 0100 0001 1100      osr=00 dig_filt=111 resXYZ = 100000 osr2=10
  }
  delay(100);
  Serial.print("\n Setup Complete!\n\n");
  delay(1000);

}

void loop() {
  //MagOne
  uint8_t MagOneReading[8];
  uint8_t MagOneStatus;
   /*********************************************************************************** 
                       SELECT SINGLE MEASUREMENT MODE
   ***********************************************************************************/
  Wire.beginTransmission(Addr);  // Start I2C
  Wire.write(0x3F); // Single measurement mode  (E=ZXYT = 1110)
  Wire.endTransmission(); // Stop I2C 
  Wire.requestFrom(Addr, 1);  // Request 1 byte of data
  if(Wire.available() == 1)  // Read status byte
  {
     MagOneStatus = Wire.read();
     //Serial.print("R>");
     //Serial.print(MagOneStatus,BIN);
     //Serial.print(": ");
  }
  delay(10);
  /*********************************************************************************** 
                       SELECT READ MEASUREMENT MODE
   ***********************************************************************************/
  Wire.beginTransmission(Addr);// Start I2C Transmission
  Wire.write(0x4F); // Read measurement command (ZXYT = 1111)
  Wire.endTransmission(); // Stop I2C Transmission
  Wire.requestFrom(Addr, 9); // Request 7 bytes of data
   /*********************************************************************************** 
                         OBTAIN 9 BYTES OF DATA
   ***********************************************************************************/
  if(Wire.available() == 9);
  {
    MagOneReading[0] = Wire.read(); //Status byte
    MagOneReading[1] = Wire.read(); //tMag msb
    MagOneReading[2] = Wire.read(); //tMag lsb
    MagOneReading[3] = Wire.read(); //xMag msb
    MagOneReading[4] = Wire.read(); //xMag lsb
    MagOneReading[5] = Wire.read(); //yMag msb
    MagOneReading[6] = Wire.read(); //yMag lsb
    MagOneReading[7] = Wire.read(); //zMag msb
    MagOneReading[8] = Wire.read(); //zMag lsb
  }
  
  
   /*********************************************************************************** 
                                COMBINING LSB AND MSB (CONCATENATION)
   ***********************************************************************************/
  int16_t xM = (((int16_t)MagOneReading[3])<<8) | MagOneReading[4];  //Mag readings two's comp
  int16_t yM = (((int16_t)MagOneReading[5])<<8) | MagOneReading[6];
  int16_t zM = (((int16_t)MagOneReading[7])<<8) | MagOneReading[8];

   static int16_t xC=0;
   static int16_t yC=0;
   static int16_t zC=0;
  /*********************************************************************************** 
                                APPLY VALUES OF GAIN = 0
   ***********************************************************************************/
  float Bx = 805* xM/100000-xC;
  float By = (805* yM/100000)-yC;
  float Bz = (2936* zM/100000)-zC;
  //uint16_t tM_alt = (uint16_t(MagOneReading[1])<<8) | MagOneReading[2]; //alt way to shift

/*********************************************************************************** 
                                  MEAN LINE =0 (APPLY)
   ***********************************************************************************/
     static int i=i;
     i++;
     if(i == 5)
   { xC = Bx;
     yC = By;
     zC = Bz;
   }  
 
    /*********************************************************************************** 
                           PLOTTING THE VALUES OF MAGNETIC FIELD OF X,Y,Z
     ***********************************************************************************/
     Serial.print(Bx);
     Serial.print(" ");
     Serial.print(By);
     Serial.print(" ");
     Serial.print(Bz); 
     Serial.print("\n");
     //uint16_t tM_alt = (uint16_t(MagOneReading[1])<<8) | MagOneReading[2]; //alt way to shift
     //Print Values for plotting
     /*********************************************************************************** 
                   PLOTTING THE VALUES OF MAGNETIC FIELD OF X,Y,Z (BEFORE GAIN)
     ***********************************************************************************/
  /*   Serial.print(xM);
     Serial.print(" ");
     Serial.print(yM);
     Serial.print(" ");
     Serial.print(zM);
     Serial.print("\n");*/   
  delay(5);

  //pressure sensor
  presVolt = analogRead(presPin);     // read the input pin
  //Serial.println(presVolt);             // debug value

    //pressure sensor calibration
    presVal = ((presVolt*256.42)-121.74)/-10; //values taken from Dushyants LV program

  //display
  display.clearDisplay();   // clears the screen and buffer
  display.setTextSize(1.5);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Pos. (x,y,z): ");
  display.setTextSize(1);
  display.setCursor(10,10);
  display.println(Bx);
  display.setCursor(10,20);
  display.println(By);
  display.setCursor(10,30);
  display.println(Bz);
  display.display();
  delay(200);

}
