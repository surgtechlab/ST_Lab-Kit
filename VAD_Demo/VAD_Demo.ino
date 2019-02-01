/* 
 *  This program has been written by James Kinch on behalf of the Surgical Technologies Laboratory
 *  at the University of Leeds.
 *  
 *  This code is written to work with the Adafruit Huzzah EPS32 Featherboard.
 *  
 *  Feel free to edit as required. 
 */

// Screen Setup
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

int numRot = 2; //setting the amount of screen rotation as the screen is rotated. 

// Software SPI (slower updates, more flexible pin options):
// pin 7 - Serial clock out (SCLK)
// pin 6 - Serial data out (DIN)
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 18, 33, 12, 27);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

//Load Cell Setup
int loadPin = 25; //set load cell analog pin input
int loadVal = 0;

  //Calibration details
  float ReadingA_Strain1 = 1840.0; //reading from sensor
  float LoadA_Strain1 = 0.0; //  (Kg,lbs..) Enter load applied
  float ReadingB_Strain1 = 3287; //reading from sensor
  float LoadB_Strain1 = 25.0; //  (Kg,lbs..) Enter load applied

//Pressure Sensor Setup
int presPin = 34; //set pressure sensor analog pin input
int presVolt = 0;
int presVal = 0;

//LED Setup
int redLED = 14;
int yellowLED = 32;
int greenLED = 15;

//Traffic Light LED Configuration - These are the green, amber and red threshold values (g,a,r)
  /*
   * From functional tests of the loads required to break the vacuum seal using the VAD device, the following loads have been determined to the the 
   * green, amber and red thresholds for the corrosponding pressures:
   * 55kPa = 0,50,70
   * 60kPa = 0,60,80
   * 65kPa = 0,70,90
   * 70kPa = 0,80,90
   * Based on these thresholds, the traffic light system is configured. 
   */
int pres_thresh[] = {50,60,70,80,90}; 
int greenAmberLED_thresh = 50;
int amberRedLED_thresh = 60;

void setup() {
  Serial.begin(9600); //  setup serial

  //LEDs Setup
  pinMode(redLED, OUTPUT); //RED LED
  pinMode(yellowLED, OUTPUT); //YELLOW LED
  pinMode(greenLED, OUTPUT); //GREEN LED

    //LED TEST
    digitalWrite(14, HIGH);
    digitalWrite(32, HIGH);
    digitalWrite(15, HIGH);
    delay(1000);
    digitalWrite(14, LOW);
    digitalWrite(32, LOW);
    digitalWrite(15, LOW);    
    delay(250);
    digitalWrite(14, HIGH);
    digitalWrite(32, LOW);
    digitalWrite(15, LOW); 
    delay(250);
    digitalWrite(14, LOW);
    digitalWrite(32, HIGH);
    digitalWrite(15, LOW); 
    delay(250);
    digitalWrite(14, LOW);
    digitalWrite(32, LOW);
    digitalWrite(15, HIGH); 
    delay(250);
    digitalWrite(14, LOW);
    digitalWrite(32, LOW);
    digitalWrite(15, LOW);    
    delay(250);
    digitalWrite(14, HIGH);
    digitalWrite(32, HIGH);
    digitalWrite(15, HIGH);
    delay(250);
    digitalWrite(14, LOW);
    digitalWrite(32, LOW);
    digitalWrite(15, LOW);    
    delay(250);

  //display init
  display.begin();
  display.setContrast(65);
  pinMode(26, OUTPUT); //LED backlight power
  dacWrite(26,255); //required to use dacWrite as analogWrite not a completed fuction in Esspresif lib.

  display.clearDisplay();   // clears the screen and buffer
  display.setRotation(numRot);  // rotate 90 degrees counter clockwise, can also use values of 2 and 3 to go further.
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
  display.setRotation(numRot);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Atraumatic");
  display.setCursor(0,10);
  display.println("Vacuum");
  display.setCursor(0,20);
  display.println("Assisted");
  display.setCursor(0,30);
  display.println("Delivery");
  display.setCursor(0,40);
  display.println("Demo");
  display.display();
  delay(2000);
}

void loop() {
  //load cell
  float newReading_Strain1 = analogRead(loadPin);  // analog in 0 for Strain 1
  //Serial.println(newReading_Strain1);  // debug value

  // Calculate load by interpolation 
  float load_Strain1 = ((LoadB_Strain1 - LoadA_Strain1)/(ReadingB_Strain1 - ReadingA_Strain1)) * (newReading_Strain1 - ReadingA_Strain1) + LoadA_Strain1;


  //pressure sensor
  presVolt = analogRead(presPin);     // read the input pin
  //Serial.println(presVolt);             // debug value

    //pressure sensor calibration
    presVal = ((presVolt*256.42)-121.74)/-10; //values taken from Dushyants LV program


  //LED TRAFFIC LIGHT SYSTEM
  //Determining the pressure and setting - pres_thresh[] = {50,60,70,80,90};
  if (presVal < 55) {
    greenAmberLED_thresh = pres_thresh[0];
    amberRedLED_thresh = pres_thresh[2];
  }
  else if (presVal < 60) {
    greenAmberLED_thresh = pres_thresh[1];
    amberRedLED_thresh = pres_thresh[3];
  }
  else if (presVal < 65) {
    greenAmberLED_thresh = pres_thresh[2];
    amberRedLED_thresh = pres_thresh[4];
  }
  else if (presVal < 70) {
    greenAmberLED_thresh = pres_thresh[3];
    amberRedLED_thresh = pres_thresh[4];
  }

  Serial.println("greenAmberLED_thresh");
  Serial.println(greenAmberLED_thresh);
  Serial.println("amberRedLED_thresh");  
  Serial.println(amberRedLED_thresh);
  
  if (load_Strain1 > amberRedLED_thresh) {
    digitalWrite(redLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, LOW);
  }
  else if (load_Strain1 <= amberRedLED_thresh && load_Strain1 >= greenAmberLED_thresh ) {
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(greenLED, LOW);
  }
  else if (load_Strain1 < greenAmberLED_thresh) {
    digitalWrite(redLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(greenLED, HIGH);
  }

  //DISPLAY
  display.clearDisplay();   // clears the screen and buffer
  display.setRotation(numRot);
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);
  display.println("Force (N):");
  display.setCursor(10,10);
  display.println(load_Strain1);
  display.setCursor(0,20);
  display.println("Pres. (kPa):");
  display.setCursor(10,30);
  display.println(presVal);
  display.display();
  delay(200);

}
