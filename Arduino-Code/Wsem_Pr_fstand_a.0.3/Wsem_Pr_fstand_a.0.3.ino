#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Wire.h>
#define DT A0
#define SCK A1
HX711 scale (DT, SCK);
byte inputs=0;
byte address=0x20; //I²C-Adresse des portexpanders für Bedienelement
byte  GPIOA=0x12;
byte  GPIOB=0x13;

//input pins
const int upButton = 5;
int upButtonCurrent=0;
int upButtonLast=0;
const int downButton = 6;
int downButtonCurrent=0;
int downButtonLast=0;
const int leftButton = 3;
int leftButtonCurrent=0;
int leftButtonLast=0;
const int rightButton = 4;
int rightButtonCurrent=0;
int rightButtonLast=0;
const int okButton = 2;
int okButtonCurrent=0;
int okButtonLast=0;
const int tareButton = 7;
int tareButtonCurrent=0;
int tareButtonLast=0;



bool debug=false;
bool LCDSimulation=true;

int menuMax=5; //anzahl an untermenüs
int menuNr=0; //aktuelles untermenü
int menuOnce=-1; //verhindert unnötiges neuzeichnen von Menümasken

//Scale
int sensorType=0; //currently used Sensor
float calibration_factor[4]=  {405959,  //Kalibrierungsfaktor 5kg-Sensor
                              40596,    //!fiktiv! Kalibrierungsfaktor 50kg-Sensor
                              0,        //Kalibrierungsfaktor 200kg-Sensor
                              0};       //Kalibrierungsfaktor weiterer Sensor
String Sensor[4]={{"5KG"},              //entsprechender sensorname
                  {"50KG"},
                  {"200KG"},
                  {"nul"}};

                              
//LCD
LiquidCrystal_I2C lcd(0x27,22,6); //addr., spalten, zeilen des Displays
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Booting...");
  Serial.println("Booting...");
  Wire.begin();
  LCDSimul("LCD initialising");
  LCDSimul("n");
  lcd.setCursor(0,0);
  lcd.print("LCD initialising");
  lcd.init();
  
  LCDSimul("set scale");
  LCDSimul("n");
  lcd.print("set scale");
  scale.set_scale();
  LCDSimul("scale tare");
  LCDSimul("n");
  lcd.print("scale tare");
  scale.tare();  
  LCDSimul("Ready!");
  LCDSimul("n");
  lcd.print("Ready");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  scale.set_scale(calibration_factor[sensorType]);
  float tmpWeight=scale.get_units(2);
  Debug();
  ButtonEvent();
  showMenu();
  //Serial.print(scale.get_units(1),3);
  //Serial.println(" kg of load");
  delay(2);
}

void ButtonEvent(){
  Wire.beginTransmission(address);
  Wire.write(GPIOB);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  inputs=Wire.read();
  Debug(String(inputs));
  //Byteshifting um die einzelnen zustände der Buttons aus dem gesammten Byte zu isolieren und ihren variablen zuzuordnen
  byte temp;
  temp=shifting(inputs,1);
  if(temp==1)
    okButtonCurrent=1;
  else if(temp==0)
    okButtonCurrent=0;
  temp=shifting(inputs,2);
  if(temp==1)
    upButtonCurrent=1;
  else if(temp==0)
    upButtonCurrent=0;
  temp=shifting(inputs,3);
  if(temp==1)
    downButtonCurrent=1;
  else if(temp==0)
    downButtonCurrent=0;
  temp=shifting(inputs,4);
  if(temp==1)
    leftButtonCurrent=1;
  else if(temp==0)
    leftButtonCurrent=0;
  temp=shifting(inputs,5);
  if(temp==1)
    rightButtonCurrent=1;
  else if(temp==0)
    rightButtonCurrent=0;
  temp=shifting(inputs,6);
  if(temp==1)
    tareButtonCurrent=1;
  else if(temp==0)
    tareButtonCurrent=0;

    
  if(tareButtonCurrent==1){
    tareButtonLast=1;
  }
  if(tareButtonCurrent==0){
    if(tareButtonLast==1){
      buttonEventTare();//tareButton Event
      tareButtonLast=0;
    }
  }
  
  if(rightButtonCurrent==1){
    rightButtonLast=1;
  }
  if(rightButtonCurrent==0){
    if(rightButtonLast==1){
      buttonEventRight();//rightButton Event
      rightButtonLast=0;
    }
  }

  if(leftButtonCurrent==1){
    leftButtonLast=1;
  }
  if(leftButtonCurrent==0){
    if(leftButtonLast==1){
      buttonEventLeft();//leftButton Event
      leftButtonLast=0;
    }
  }

  if(upButtonCurrent==1){
    upButtonLast=1;
  }
  if(upButtonCurrent==0){
    if(upButtonLast==1){
      buttonEventUp();//upButton Event
      upButtonLast=0;
    }
  }

  if(downButtonCurrent==1){
    downButtonLast=1;
  }
  if(downButtonCurrent==0){
    if(downButtonLast==1){
      buttonEventDown();//downButton Event
      downButtonLast=0;
    }
  }
}

void buttonEventOk(){
  
}
void buttonEventUp(){
  
}
void buttonEventDown(){
  
}
void buttonEventLeft(){
  menuMinus();
}
void buttonEventRight(){
  menuPlus();
}
void buttonEventTare(){
   scale.tare();
   Debug("tare");
}

void menuPlus(){
    menuNr++;
  if(menuNr >=menuMax)
    menuNr=0;
}
void menuMinus(){
    menuNr--;
  if(menuNr<0)
    menuNr=menuMax-1;
}

void showMenu(){
  switch (menuNr){
    case 0:
      menu0();
      break;
    case 1:
      menu1();
      break;
    case 2:
      menu2();
      break;
    case 3:
      menu3();
      break;
    case 4:
      menu4();
      break;
  }
}

void menu0(){
  if(menuOnce!=0){
    Serial.println("MENUAONCE");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Waage");
    lcd.setCursor(18,0);
    lcd.print("0");
    lcd.setCursor(0,1);
    lcd.print("Gewicht|Typ|Rohdaten");
    lcd.setCursor(6,2);
    lcd.print("Kg");
    lcd.setCursor(9,2);
    lcd.print(Sensor[sensorType]);
    lcd.setCursor(0,3);
    lcd.print("<Tara um zu nullen >");
    menuOnce=0;
  }
  lcd.setCursor(0,2);
  lcd.print(scale.get_units(2), 3);
  lcd.setCursor(14,2);
  lcd.print(scale.get_value(),0);

  LCDSimul("Waage     0");
  LCDSimul("n");
  LCDSimul("Gewicht|Typ|Rohdaten");
  LCDSimul("n");
  LCDSimul(String(scale.get_units(2), 3));
  LCDSimul(" ");
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul(" ");
  LCDSimul(String(scale.get_value(),0)); 
  LCDSimul("n"); 
  LCDSimul("<Tara um zu nullen >");
  LCDSimul("n");    
}
void menu1(){
  LCDSimul("Sensor auswählen    1");
  LCDSimul("n");
}
void menu2(){
  LCDSimul("Sensor kalibrieren    2");
  LCDSimul("n");
}
void menu3(){
  LCDSimul("Motorsteuerung    3");
  LCDSimul("n");
}
void menu4(){
  LCDSimul("Klebstoffprüfung    4");
  LCDSimul("n");  
}

byte shifting(byte input, int isol){//isol: zu isolierender Bit
  byte tmp=input<<(8-isol);
  tmp=tmp>>7;
  return tmp;
}

void LCDWrite(String in,int x,int y){ //Schreibt einen String an angegebenen x-y-koordinaten auf das LCD Display
  //lcdErsatz[x]=in;
  lcd.setCursor(x,y);
  lcd.print(in);
  /*for(int i=0;i<4;i++){
    Serial.println(lcdErsatz[i]);
  }*/
  
}

void Debug(){
  if(debug){
    if(Serial.available()){
      char temp = Serial.read();
      if(temp=='d')
        menuPlus();
      else if(temp=='a')
        menuMinus();
    }
    Serial.print(upButtonCurrent);
    Serial.print("  ");
    Serial.print(downButtonCurrent);
    Serial.print("  ");
    Serial.print(leftButtonCurrent);
    Serial.print("  ");
    Serial.print(rightButtonCurrent);
    Serial.print("  ");
    Serial.print(okButtonCurrent);
    Serial.print("  ");
    Serial.print(tareButtonCurrent);
    Serial.print("  ");
    Serial.print(menuNr);
    Serial.print("  ");
    Serial.println();
  }
}

void Debug(String in){
  if(debug){
    Serial.println(in);
  }
}

void LCDSimul(String in){
  if(LCDSimulation==true){
    String comp="n";
    if(in==comp){
      Serial.println(" ");
    }
    else{
      Serial.print(in);
    }
  }
}

