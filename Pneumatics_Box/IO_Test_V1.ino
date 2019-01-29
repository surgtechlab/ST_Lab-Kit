/*
 * Pnuematic control rig I/O test program
 * 
 * This program was developed to test the function of the required I/O for a pnuematic control rig being developed. 
 * Each of the components controls are broken down into constituent parts for ease of understanding and programming.
 * 
 * Sections of this code can be directly lifted and written into a complete control program the fulfils the full
 * operational requirements of the pnuematic control rig. 
 * 
 * This test code was written by James Kinch on behalf of the Surgical Technologies Lab @ the University of Leeds.
 * Date: 24/08/2018
 * 
 * NOTE: no timing has been implemented into this program. As a result, all of the DAQ is completely no deterministic. 
 */
// It is assumed that the LCD module is connected to
// the following pins.
//      SCK  - Pin 8
//      MOSI - Pin 9
//      DC   - Pin 10
//      RST  - Pin 11
//      CS   - Pin 12
//

#include <LCD5110_Basic.h>

LCD5110 myGLCD(76,75,10,11,12); //setting screen pics

extern uint8_t SmallFont[];

//def pressure sensor variables
int presSensPin[] = {3,4,5,6}; //pressure sensor pins
double presSensRead[] = {0,0,0,0}; //pressure sensor 1, 2, 3, and 4.

//def joystick varialbes
int up_button = 2;
int down_button = 4;
int left_button = 5;
int right_button = 3;
int start_button = 6;
int select_button = 7;
int joystick_button = 8;
int joystick_axis_x = A0;
int joystick_axis_y = A1;
int buttons[] = {up_button, down_button, left_button, right_button, start_button, select_button, joystick_button};

//def mosfet variables
int mosfetPin1 = 23;
int mosfetPin2 = 25;
int mosfetPin3 = 27;
int mosfetPin4 = 29;
int mosfetPin[] = {mosfetPin1, mosfetPin2, mosfetPin3, mosfetPin4}; //pressure sensor pins
boolean mosfetWrite[] = {false,false,false,false}; //mosfet 1, 2, 3, and 4 (control the valves).

//def regulator variables
int regMonitorPin = 56; //A2 INPUT
int regControlPin = 66; //DAC0 OUTPUT
double regMonitorVolt = 0;
double regMonitorPres = 0;
double regControlVolt = 0;
double regControlPSI = 0;


void setup()
{
  Serial.begin(9600); //initialising serial read out for debugging

  //initialising LCD screen 
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);
  
  //initialising mosfet pins
  for (int i=0; i < 4; i++)
  {
   pinMode(mosfetPin[i], OUTPUT);
   //Serial.println(mosfetPin[i]);
  }
  
  //initialising joystick buttons
  for (int i=0; i < 7; i++)
  {
   pinMode(buttons[i], INPUT);
   digitalWrite(buttons[i], HIGH);
  }

  //intialising regulator pins
  pinMode(regControlPin, OUTPUT); //the monitor is only an analog input so does not need setting.
  
}

void loop()
{
  //pressure sensor test
  for (int i=0; i <= 3; i++) {
    presSensRead[i] = analogRead(presSensPin[i]);
    //Serial.println(presSensRead[i]);
  }

  //joystick test
//  Serial.print("UP = "),Serial.print(digitalRead(up_button)),Serial.print("\t");
//  Serial.print("DOWN = "),Serial.print(digitalRead(down_button)),Serial.print("\t");
//  Serial.print("LEFT = "),Serial.print(digitalRead(left_button)),Serial.print("\t");
//  Serial.print("RIGHT = "),Serial.print(digitalRead(right_button)),Serial.print("\t");
//  Serial.print("START = "),Serial.print(digitalRead(start_button)),Serial.print("\t");
//  Serial.print("SELECT = "),Serial.print(digitalRead(select_button)),Serial.print("\t");
//  Serial.print("ANALOG = "),Serial.print(digitalRead(joystick_button)),Serial.print("\t");
//  Serial.print("X = "),Serial.print(map(analogRead(joystick_axis_x), 0, 1000, -1, 1));Serial.print("\t");
//  Serial.print("Y = "),Serial.print(map(analogRead(joystick_axis_y), 0, 1000, -1, 1));Serial.print("\n");  
//  Serial.print("X = "),Serial.print(analogRead(joystick_axis_x));Serial.print("\t");
//  Serial.print("Y = "),Serial.print(analogRead(joystick_axis_y));Serial.print("\n");

  //mosfet test - 4 buttons of the joystick control the actuation of the mosfets. 
  digitalWrite(mosfetPin[0], !digitalRead(up_button));
  digitalWrite(mosfetPin[1], !digitalRead(right_button));
  digitalWrite(mosfetPin[2], !digitalRead(down_button));
  digitalWrite(mosfetPin[3], !digitalRead(left_button));

  //regulator test
  regMonitorVolt = analogRead(regMonitorPin);
  regMonitorPres = (regMonitorVolt * 2.898) + 0.0392; //calibration equation to move from reading of voltage to PSI.
  Serial.println(regMonitorPres);
  
  regControlPSI = 1;
  regControlVolt = (regControlPSI-0.0392)/2.898;
  analogWrite(regControlPin, 5);
  
  //screen test
  myGLCD.clrScr();
  myGLCD.print("Pressure Values", LEFT, 0);
  myGLCD.printNumF(presSensRead[0], 2, CENTER, 8);
  myGLCD.printNumF(presSensRead[1], 2, CENTER, 16);
  myGLCD.printNumF(presSensRead[2], 2, CENTER, 24);
  myGLCD.printNumF(presSensRead[3], 2, CENTER, 32);
  delay (100);

}
