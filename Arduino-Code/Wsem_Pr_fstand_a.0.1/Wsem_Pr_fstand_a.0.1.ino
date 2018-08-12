#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Wire.h>
#define DT A0
#define SCK A1
HX711 scale (DT, SCK);
//input pins
const int upButton = 5;
const int downButton = 6;
const int leftButton = 3;
const int rightButton = 4;
const int okButton = 2;
const int tareButton = 7;
bool debug=false;

//Scale
int sensorType=0; //currently used Sensor
float calibration_factor[4]={405959,0,0,0};
//LCD
LiquidCrystal_I2C lcd(0x27,22,6);
String lcdErsatz[4]={{},
                    {},
                    {},
                    {}}; //String resebles LCD output for use without LCD-Screen; debug needs to be "true" to work

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Booting...");
  Serial.println("Booting...");
  pinMode(upButton,INPUT);
  pinMode(downButton,INPUT);
  pinMode(leftButton,INPUT);
  pinMode(rightButton,INPUT);
  pinMode(okButton,INPUT);
  pinMode(tareButton,INPUT);
  Serial.println("LDC initialising");
  lcd.init(); 
  Serial.println("set scale");
  scale.set_scale();
  Serial.println("scale tare");
  scale.tare();  
  Serial.println("Ready!");
}

void loop() {
  // put your main code here, to run repeatedly:
  scale.set_scale(calibration_factor[sensorType]);
  float tmpWeight=scale.get_units(2);
  Debug();
  ButtonEvent();
  Serial.print(scale.get_units(1),3);
  Serial.println(" kg of load");
  delay(2);
}

void ButtonEvent(){
  if(digitalRead(tareButton)==HIGH){
    scale.tare();
    Serial.println("tare");
  }
}

void LCDPrint(String in,int x,int y){
  lcdErsatz[x]=in;
  lcd.setCursor(x,y);
  lcd.print(in);
  for(int i=0;i<4;i++){
    Serial.println(lcdErsatz[i]);
  }
  
}
void Debug(){
  if(debug){
    Serial.print(digitalRead(upButton));
    Serial.print("  ");
    Serial.print(digitalRead(downButton));
    Serial.print("  ");
    Serial.print(digitalRead(leftButton));
    Serial.print("  ");
    Serial.print(digitalRead(rightButton));
    Serial.print("  ");
    Serial.print(digitalRead(okButton));
    Serial.print("  ");
    Serial.print(digitalRead(tareButton));
    Serial.print("  ");
    Serial.println();
  }
}

