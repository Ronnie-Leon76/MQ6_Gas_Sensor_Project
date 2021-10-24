//These are the global variables for the program.
const int buzzerPin=8;
const int motorPin=9;
const int digitalGas=7;
const int analogGas=A0; 
const int gsmReceiver=2; 
const int gsmTransmitter=3;

#include <String.h>

//These are the libraries for the OLED module.
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>

//Creating an object of the Adafruit_SSD1306
Adafruit_SSD1306 disp(-1); // send -1 to the constructor so that none of the arduino pins is used as a reset for the display

//The Software Serial will be used to enable serial communication using digital pins of the Arduino
#include <SoftwareSerial.h>
//Create a software serial object to communicate with SIM800L
SoftwareSerial mySerial(gsmTransmitter, gsmReceiver); //SIM800L Tx & Rx is connected to Arduino digital pins.


void setup() 
{
  pinMode(buzzerPin, OUTPUT);
  pinMode(motorPin, OUTPUT);
  //Initialize the I2C address 0x3C
  disp.begin(SSD1306_SWITCHCAPVCC, 0X3C);
  gsmSetup();
}

//Function to switch on the buzzer and keep it pulsating.
void buzzerOn()
{
  while(1)
  {
    digitalWrite(buzzerPin, HIGH);
    delay(1000);
    digitalWrite(buzzerPin, LOW);
    delay(1000);
  }
}

//Function to switch off the buzzer.
void buzzerOff()
{
  while(1)
  {
    digitalWrite(buzzerPin, LOW);
  }
}

//Function to read the leakage value.
int readLeakage()
{
  int leakValue;
  leakValue=analogRead(analogGas);
  value = map(leakValue, 0, 1023, 0, 100)
  return value;
}

//Function to control a motor.
void activateMotor(int value)
{
  int motorSpeed;
  motorSpeed=value/4;
  analogWrite(motorPin, motorSpeed);
  delay(4000);
  analogWrite(motorPin, 0);
}

//Function for the OLED display.
void OLED_Display(int leakPercent, String comment)
{
  //First, clear the buffer.
   disp.clearDisplay();
   disp.setTextSize(1);
   disp.setTextColor(WHITE);
   disp.setCursor(1,28);
   disp.println("Reading values...");
   //disp.display();
   delay(500);
   disp.print("Leakage(%): ");
   //disp.display();
   disp.print(leakPercent);
   //disp.display();
   disp.print("Status: %s");
   disp.print(comment);
   disp.display();
}

void gsmSetup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();

  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  
  mySerial.println("AT+CMGS=\"+254727965941\"");//The phone number to receive the messages.
  updateSerial();
}

//The update serial function for the GSM module.
void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

//Function for the GSM module communication with the registered user.
void gsmCommunication(int leakPercent)
{
  if ((leakPercent>40) && (leakPercent<=60))
  { 
    mySerial.println("Your LPG is leaking. The leakage status is bad. The system has started solving the problem.");
    delay(100);
    updateSerial();
    mySerial.println((char)26);
    delay(1000);
  }
  else if (leakPercent>60)
  {
    mySerial.print("Your LPG is leaking. The leakage status is critical. The system has started solving the problem.");
    updateSerial();
    mySerial.write(26);
  }
}

void solutionMessage()
{
  mySerial.print("The system has solved the LPG leakage. We keep you updated.");
  mySerial.write(26);
}

String commentStatus(int leakPercent)
{ 
  String comment; 
  if ((leakPercent>0)&&(leakPercent<=10))
  {
    comment="Negligible"; 
  }
  else if ((leakPercent>10)&&(leakPercent<=40))
  {
    comment="Normal";
  }
  else if ((leakPercent>40)&&(leakPercent<=60))
  {
    comment="Bad";
  }
  else if ((leakPercent>60)&&(leakPercent<=100))
  {
    comment="Critical";
  }
  else
  {
    comment="None";
  }
  return comment;
}

//Crash message
void crashMessage()
{
  mySerial.print("The system was unable to solve the leakage. We request you to check out the issue.");
  mySerial.write(26);
}
void loop() 
{
  int leakVal, leakPercentage;
  String leakComment;
  delay(1000);
  leakVal=readLeakage();
  leakPercentage=(leakVal/1024)*100;
  leakComment=commentStatus(leakPercentage);

  OLED_Display(leakPercentage, leakComment);
  
  if (leakPercentage>40)
  {
    buzzerOn();
    activateMotor(leakVal);
    gsmCommunication(leakPercentage);
    leakVal=readLeakage();
    leakPercentage=(leakVal/1024)*100;
    if (leakPercentage<40)
    {
      solutionMessage();
      buzzerOff();
    }
    else if (leakPercentage>40)
    {
      activateMotor(leakVal);
      gsmCommunication(leakPercentage);
      leakVal=readLeakage();
      leakPercentage=(leakVal/1024)*100;
      if (leakPercentage<40)
      {
        solutionMessage();
        buzzerOff();
      }
      else if (leakPercentage>40)
      {
        activateMotor(leakVal);
        gsmCommunication(leakPercentage);
        leakVal=readLeakage();
        leakPercentage=(leakVal/1024)*100;
        if (leakPercentage<40)
        {
          solutionMessage();
          buzzerOff();
        }
        else
        {
          crashMessage();
        }
      }
    }
  }
}
