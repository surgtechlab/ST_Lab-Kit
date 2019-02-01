/*
Copyright (c) 2018  All rights reserved.
Surgical Technologies Research Group, School of Mechanical Engineering, University of Leeds;
 * Bespoke Film Applicator
 * Rewritten and Updated by: Jun Wai Kow- SurgicalTech-Lab; University of Leeds
 * Last Update Date: 15/1/2018
 * 
 * NOTE: no innate timing has been implemented into this program apart from the microprocessor's innate clock
 *
// BOM:
 -16x2 LCD module (Pins 7, 6, 5, 4, 3, 2). Library is innate within Arduino hardware.
 -AnalogButton Configuration- (Pin Analog5). 
 [Resistor Values 2k, 330ohm, 660ohm, 1k ohm, 3.6k ohm, 4.7k ohm; (list out the resistors)
 -LimitSwitches - (Pins 8 and 9 DIO)
 -MotorDriver DM5420- (Pulse 13,Direction 12,Enable 11)
 
 //
 Functionality:
 Applied to the bespoke applicator set up, the intrest of the hardware motion is to go from left to right, 
 while able to change SPEED of the motors and have an active stop button. (MM/S) speed of the motors
 Additional set up can be added such as a quick homing sequence of either sides, but main functionality would be:
 
Up/Down -Change the Speed
Left/Right -Move to the intended direction at the set speed.
*/

//Delcare LCD (DIO)
#include <LiquidCrystal.h>	//Innate Arduino Library
const int rs = 7, en = 6, d4 = 5, d5 = 4, d6 = 3, d7 = 2;	//Relative to the LCD pins, these are the connections
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); //assign Pins and point to the library
//byte skull[8] = {72, 54, 93, 73, 91, 109, 64, 41}; //SkullShaped Pixel byte
byte heart[8] = {0, 0, 27, 31, 31, 14, 4, 0}; //Icon to show position of the gantry (no actual positioning calculation); //HearShaped Pizel byte

//Declare MotorDriver (DIO)	//MotorDriver DM2542 //Driving a Nema17 motor
int pulsePin = 13; int directPin = 12; int enablePin = 11; //PinAssignment-Reflect to the datasheet of the motorDriver
int motSpeed = 1; // Speed unit mm/s; Set towards min: 1mm/s while normal use threshold would be 10mm/s
int minMS = 0;
int maxMS = 50;	//Max set as 5cm/s applicable for other linear stage usages

//Declare LimitSwitches (DIO)
const int lsw0 = 8, lsw1 = 9;	//Limit Swith Pins
int ls0 = 0, ls1 = 0; //PullDown Pins variables

//DeclareButtons (Analog Layout)
//[Resistor Values 2k, AI-signal- RIGHT; 330ohm, UP; 660ohm, DOWN; 1k ohm, LEFT; 3.6k ohm, ENTER;  4.7k ohm; ESC (list out the resistors)
//Declare Buttons (Connected to Analog);
#define upB 1
#define downB 2
#define rightB 3
#define leftB 4
#define startB 5
#define selB 6
#define noB 7   //defaultValue
int buttonLog = A5; //AnalogPin to readButtons (Any analog button will do)

//------------------------------------------------------------------------
//Program Start
void setup()
{ 
//Initialise MotorStepper
  pinMode(pulsePin, OUTPUT);
  pinMode(directPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  //Initialise //Delay value is suggested in the DM542 datasheet
  digitalWrite(enablePin, LOW); delay(10);
  digitalWrite(directPin, LOW); delay(10);
  digitalWrite(pulsePin, LOW); delay(10);
  
//Initialise LCD
  lcd.begin(16,2); //RowColumn  
  lcd.setCursor(0,0);	//setCursor
  lcd.print("Initialising ...");	//TestPrint (Wont be able to see this, but, hey.. try)
  delay(250);
  lcd.clear();	//Clear the screen
  lcd.createChar(0, heart);	//Initialise the byte icon

//Initialising buttons
  pinMode(buttonLog, INPUT); //Set the Pin to AnalogRead
  pinMode(lsw0, INPUT); //Digital Output; True -> False
  pinMode(lsw1, INPUT);
  digitalWrite(lsw0,HIGH);  //LimitSwitches are always HIGH, if pressed becomes LOW
  digitalWrite(lsw1,HIGH);
}//EndSetup

void loop()
{ 
  displayConst();
  delay(250); //Debounce Period for buttonlogging
//Command check
  for(int i = 0; i < 10000; i++)    //Checks for Button responses
  {
    buttonLoop(); //check for button pushed
  }

}//EndLoop

int readJoystick()  //ReadJoystick Fx
{ 
  int adcButtons = analogRead(buttonLog); 
  if (adcButtons > 1000) return noB; //1st option for speed reasons since it will be the most likely result _(Default_Floating)
  if (adcButtons < 50)   return rightB; 
  if (adcButtons < 195)  return upB;
  if (adcButtons < 380)  return downB;
  if (adcButtons < 560)  return leftB;
  if (adcButtons < 790)  return startB;  
  if (adcButtons < 980)  return selB; 
  return noB;  // When all others fail, return this... (Awaiting for return Val when button pressed)
  
}//End-readJoystick

void lcdClr() //Reset/Clear screen Function;
{               
        //Refresh to normal states
        lcd.clear();    //Clear screen
        lcd.setCursor(0,0);             //setCursor
        delay(100);     //Delay time to reduce data being clogged up.
}


//------------------------------------------------------------------------
//Main Accessible Functions

void buttonLoop() //Fx for buttons initiation
{
/*
ButtonLoop-automatically calls for Direct Motion & Speed Change
*/
  //Check responses
  int button = readJoystick();
  ls0 = digitalRead(lsw0); ls1 = digitalRead(lsw1); 
  int incSpeed = 1;	//
  
  delay(100);
    if(button == startB) //StartButton
    {//Acts as a reset button to refresh everything on the screen and the global values
		lcdClr();
		motSpeed = 1;
		displayConst();
		delay(100);
		//Serial.print("MenuFx"),Serial.print("\n");
    }  
//Up Down Controls the Speed    
    if(button == upB)
    {
		delay(50);
		motSpeed = motSpeed + incSpeed;
		if(motSpeed >= maxMS) motSpeed = maxMS;
		if(motSpeed < 10){lcd.setCursor(7,1); lcd.print("   ");}
		lcd.setCursor(7,1); lcd.print(" ");
		lcd.print(motSpeed);
		//if(motSpeed < 100){lcd.setCursor(11,1); lcd.print(" ");}		
    }//End-IfUpB
    
    if(button == downB)
    {
		delay(50);
		motSpeed = motSpeed - incSpeed;
		if(motSpeed <= minMS ) motSpeed = minMS;
		if(motSpeed < 10){lcd.setCursor(7,1); lcd.print("   ");}
		lcd.setCursor(7,1); lcd.print(" ");
		lcd.print(motSpeed);
    }//End-IfDownB
    
//Left Right Controls the Direction of motion
    if(button == rightB)
    {
		digitalWrite(directPin, HIGH);
		int moveN = 2;
		int spd = motSpeed*50;
		float nSteps = (1/float(spd))/2;
		while(moveN > 1)
		{ 
		  //CheckResponse
		  ls1 = digitalRead(lsw1);
		  button = readJoystick();
		  //Run Motor
			digitalWrite(pulsePin, LOW);
			delayMicroseconds(nSteps*1000000);
			digitalWrite(pulsePin, HIGH);
			delayMicroseconds(nSteps*1000000);
		  if(button == selB || ls1 == false) //EXIT
		  {
			moveN = 0; delay(100);
			break;
		  }
		}
    }//End-IfRightBB
    
    if(button == leftB)
    {
		digitalWrite(directPin, LOW);
		int moveN = 2;
		int spd = motSpeed*50;
		float nSteps = (1/float(spd))/2;
		while(moveN > 1)
		{ 
		  //CheckResponse
		  ls0 = digitalRead(lsw0);
		  button = readJoystick();
		  //Run Motor
			digitalWrite(pulsePin, LOW);
			delayMicroseconds(nSteps*1000000);
			digitalWrite(pulsePin, HIGH);
			delayMicroseconds(nSteps*1000000);
		  if(button == selB || ls0 == false) //EXIT
		  {
			moveN = 0; delay(100);
			break;
		  }
		}        
    }//End-IfLeftB

    if(button == selB)  //BackButton-(More like a refresh button)
    {
		lcdClr();
		delay(100);
		digitalWrite(pulsePin, LOW);
		loop();
    }
  
/*    //Debug LimitSwitches  
  if(ls0 == false) //RightSide
  {
    lcd.setCursor(16,0); lcd.write(byte(0));
    lcd.setCursor(0,0); lcd.write("  ");
  }
  else if(ls1 == false) //LeftSide
  {
    lcd.setCursor(0,0); lcd.write(byte(0));
    lcd.setCursor(16,0); lcd.write(" ");
  }
  else if(ls0 == false && ls1 == false) //In between
  {
    lcd.setCursor(0,0); lcd.write(" ");
    lcd.setCursor(16,0); lcd.write(" ");
  }

    if(ls0 == false or ls1 == false)
    {
    delay(100);
    lcdClr();
    lcd.print("LimitSwitch");
    }
*/  
}//End-buttonLoop


void displayConst(){
  //Display on Screen (actively Displays the Set Speed and the Location of the Gantry.
  //lcd.setCursor(0,0); lcd.write(byte(0));
  lcd.setCursor(0,0); lcd.write(byte(0));
  lcd.setCursor(4,0); lcd.print("Position");
  lcd.setCursor(0,1); lcd.print("Speed:");
  lcd.setCursor(11,1); lcd.print("mm/s");
  lcd.setCursor(7,1);  lcd.print(" "); 
  lcd.print(motSpeed);
  //ls0 = digitalRead(lsw0); ls1 = digitalRead(lsw1); 
  return;
}
