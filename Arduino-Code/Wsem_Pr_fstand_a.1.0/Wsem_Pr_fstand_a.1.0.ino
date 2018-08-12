/*
 * Software zur Verwendung mit einem Klebstoff-Prüfstand
 * Autor: Paul Sänger
 * 
 * 
 * Anschlussanweisungen:
 *    HX711 Modul:
 *      Wägezelle zu HX711:
 *        Rot     -   E+
 *        Schwarz -   E-
 *        Grün    -   A-
 *        Weiß    -   A+
 *      Modul zu Arduino:
 *        GND     -   GND
 *        VCC     -   5V
 *        DT      -   Definierter Arduino Pin DT
 *        SCK     -   Definierter Arduino Pin SCK
 *    Bedienelement:
 *      MCP23017 zu Arduino:
 *        Pin 9 |VDD    -   5V
 *        Pin 18|RESET  -   5V
 *        Pin 10|VSS    -   GND
 *        Pin 12|SCL    -   A5|SCL
 *        Pin 13|SDA    -   A4|SDA
 *        I²C-Addresse über Pin 15 - Pin 17 am MCP23017 durch verbinden mit GND oder 5V, alle 3 zu GND: Adresse 0x20/100000 
 *      Taster zu MCP23017:
 *        Über Pulldown-Wiederstände geschaltet: 5V an ein Buttonterminal, Verbindung zu MCP23017 an anderes Terminal + Verbindung zu  GND über hochohmigen Wiederstand.
 *        OK    -   GPB0|Pin 1
 *        UP    -   GBP1|Pin 2
 *        DOWN  -   GBP2|Pin 3
 *        LEFT  -   GBP3|Pin 4
 *        RIGHT -   GBP4|Pin 5
 *        TARE  -   GBP5|Pin 6
 */

#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Wire.h>
#define DT 5
#define SCK 6
HX711 scale (DT, SCK);
byte inputs=0;
byte address=0x20; //I²C-Adresse des portexpanders für Bedienelement
byte  GPIOA=0x12;
byte  GPIOB=0x13;
const int ops=4;
unsigned long previousMillis[ops] = {};  //Speicher für Zeiteinheiten von zeitgesteuerten Event (z.B.: langes drücken einer Taste)
unsigned long currentMillis;
int doonceUp=0;   //nötig um langes Halten einer Taste zu registieren
int doonceDown=0; //nötig um langes Halten einer Taste zu registieren
int doonceOk=0;   //nötig um langes Halten einer Taste zu registieren
boolean longUp=false;   //Ergebniss ob   Up lange gedrückt wurde
boolean longDown=false; //Ergebniss ob Down lange gedrückt wurde
boolean longOk=false;   //Ergebniss ob   Ok lange gedrückt wurde
int longPress=600; //Intervall in millisekunden, ab dem ein Tastendruck als lang gilt
int dirakt=0; //aktuelle Richtung des Motors
int belastungstest=0; //status des belastungstest


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

const int motorL=A3;
const int motorR=A2;



bool debug=true;
bool LCDSimulation=true;
bool SerialExchange=true;

int menuMax=5; //anzahl an untermenüs
int menuNr=0; //aktuelles untermenü
int TestPhase=0; //
int menuOnce=-1; //verhindert unnötiges neuzeichnen von Menümasken

//Scale
float tmpWeight; //Speichert die Last auf der Zelle in kg für einen Programmzyklus für höhere Performanz des Codes
float tmpValue;  //Speichert die Rohdaten-Tarafaktor der Zelle für einen Programmzyklus für höhere Performanz des Codes
float tmpRaw;    //Speichert die Rohdaten der Zelle für einen Programmzyklus für höhere Performanz des Codes
float tmpOffset;  // ---"---
const int sens = 6; //Anzahl an eigespeicherten Sensoren
int sensorType=0; //aktuell verwendeter Sensor
int sensorTypeTemp=0; //zwischenspeicher bei Sensorwechsel
float calGew=0; //Kalibrierungsgewicht
float calibration_factor[sens]=  {-405959,  //Kalibrierungsfaktor 5kg-Sensor
                              40596,    //!fiktiv! Kalibrierungsfaktor 50kg-Sensor
                              0,        //Kalibrierungsfaktor 200kg-Sensor
                              0};       //Kalibrierungsfaktor weiterer Sensor
String Sensor[sens]={{"5KG"},              //entsprechender sensorname
                  {"50KG"},
                  {"200KG"},
                  {"usr01"},
                  {"usr02"},
                  {"usr03"}};

                              
//LCD
LiquidCrystal_I2C lcd(0x27,22,6); //addr., spalten, zeilen des Displays
void setup() {
  Serial.begin(2000000);// Übertragungsgeschwindigkeit an den PC (Standard: 9600; Schnellstes: 2000000)
  lcd.init();
  lcd.setCursor(0,0);
  lcd.print("Booting...");
  Serial.println("Booting...");
  Wire.begin();
  /*Wire.beginTransmission(address);  //setzen der GPIOB outputs des I/O expanders als outputs
  Wire.write(0x01); // IODIRB register
  Wire.write(0x00); // set all of port A to outputs
  Wire.endTransmission();*/
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
  lcd.print("loading");
  LCDSimul("loading");
  LCDSimul("n");
  pinMode(motorL,OUTPUT);
  pinMode(motorR,OUTPUT);
  digitalWrite(motorL,LOW);
  digitalWrite(motorR,LOW);  
  LCDSimul("Ready!");
  LCDSimul("n");
  lcd.print("Ready");
  
}

void loop() {
  // put your main code here, to run repeatedly:
  scale.set_scale(calibration_factor[sensorType]);
  tmpRaw=scale.read_average(1);
  tmpOffset=scale.get_offset();
  tmpValue=tmpRaw-tmpOffset;
  tmpWeight=tmpValue/scale.get_scale();
  Debug();
  SerialReceive();
  ButtonEvent();
  showMenu();
  Belastungstest();
  motor();
  SerialSend();
  //Serial.print(scale.get_units(1),3);
  //Serial.println(" kg of load");
  currentMillis = millis();
  Serial.print("                                              ");
  Serial.println(currentMillis-previousMillis[0]);
  previousMillis[0]=currentMillis;
  Serial.println(dirakt);
  delay(5);
}

void ButtonEvent(){ //Aufschlüsseln des GPIOB Inputs, vorbereiten, managen und zur Verfügung stellen der Button Inputs
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

  if(okButtonCurrent==1){
    okButtonLast=1;
    if(doonceOk==0){
      previousMillis[3]=currentMillis;
      doonceOk=1;
    }
  }
  if(okButtonCurrent==0){
    if(okButtonLast==1){
      if(currentMillis-previousMillis[3]>=longPress){
        longOk=true;
      }
      else{
        longOk=false;
      }
      buttonEventOk();//OkButton Event
      okButtonLast=0;
      doonceOk=0;
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
    if (doonceUp==0){
      previousMillis[1]=currentMillis;
      doonceUp=1;
    }
  }
  if(upButtonCurrent==0){
    if(upButtonLast==1){
      if(currentMillis-previousMillis[1]>=longPress){
        longUp=true;
      }
      else{
        longUp=false;
      }
      buttonEventUp();//upButton Event
      upButtonLast=0;
      doonceUp=0;
    }
  }

  if(downButtonCurrent==1){
    downButtonLast=1;
    if(doonceDown==0){
      previousMillis[2]=currentMillis;
      doonceDown=1;
    }
  }
  if(downButtonCurrent==0){
    if(downButtonLast==1){
      if(currentMillis-previousMillis[2]>=longPress){
        longDown=true;
      }
      else{
        longDown=false;
      }
      buttonEventDown();//downButton Event
      downButtonLast=0;
      doonceDown=0;
    }
  }
}

void buttonEventOk(){
  switch (menuNr){
    case 0:
      menu0EventOk();
      break;
    case 1:
      menu1EventOk();
      break;
    case 2:
      menu2EventOk();
      break;
    case 3:
      menu3EventOk();
      break;
    case 4:
      menu4EventOk();
      break;
  }
}
void buttonEventUp(){
  switch (menuNr){
    case 0:
      menu0EventUp();
      break;
    case 1:
      menu1EventUp();
      break;
    case 2:
      menu2EventUp();
      break;
    case 3:
      menu3EventUp();
      break;
    case 4:
      menu4EventUp();
      break;
  }
}
void buttonEventDown(){
  switch (menuNr){
    case 0:
      menu0EventDown();
      break;
    case 1:
      menu1EventDown();
      break;
    case 2:
      menu2EventDown();
      break;
    case 3:
      menu3EventDown();
      break;
    case 4:
      menu4EventDown();
      break;
  }  
}
void buttonEventLeft(){
  menuMinus();
}
void buttonEventRight(){
  menuPlus();
}
void buttonEventTare(){
    switch (menuNr){
    case 0:
      menu0EventTare();
      break;
    case 1:
      menu1EventTare();
      break;
    case 2:
      menu2EventTare();
      break;
    case 3:
      menu3EventTare();
      break;
    case 4:
      menu4EventTare();
      break;
  }  
   Debug("tare");
}

void menu0EventOk(){
  
}
void menu1EventOk(){
  sensorType=sensorTypeTemp;
}
void menu2EventOk(){
  if(calGew!=0){
    if(longOk==false){
      calibration_factor[sensorType]=scale.get_value(10)/calGew;
    }
  }
  if(longOk==true){
      //neuen calibration factor im EEPROM Speichern
    }
}
void menu3EventOk(){
  dirakt=0;
}
void menu4EventOk(){
  belastungstest=1;
}

void menu0EventUp(){
  
}
void menu1EventUp(){
  sensorTypeTemp++;
  if(sensorTypeTemp>=sens){
    sensorTypeTemp=0;
  }
}
void menu2EventUp(){
  if(longUp==false){
    calGew=calGew+0.0010;
  }
  if(longUp==true){
    calGew=calGew+0.1;
  }
}
void menu3EventUp(){
  dirakt=1;
}
void menu4EventUp(){
  
}

void menu0EventDown(){
  
}
void menu1EventDown(){
  sensorTypeTemp--;
  if(sensorTypeTemp<=-1){
    sensorTypeTemp=sens-1;
  }
}
void menu2EventDown(){
  if(longDown==false){
    calGew=calGew-0.0010;
  }
  if(longDown==true){
    calGew=calGew-0.1;
  }
  if(calGew<0){
    calGew=0;
  }
}
void menu3EventDown(){
  dirakt=-1;
}
void menu4EventDown(){
  
}

void menu0EventTare(){
  scale.tare();
}
void menu1EventTare(){
  scale.tare();
}
void menu2EventTare(){
  scale.tare(10);
  Serial.println("                  tare 10");
}
void menu3EventTare(){
  scale.tare();  
}
void menu4EventTare(){
  scale.tare();  
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
  lcd.print(tmpWeight, 3);
  lcd.setCursor(14,2);
  lcd.print(tmpValue,0);

  LCDSimul("Waage     0");
  LCDSimul("n");
  LCDSimul("Gewicht|Typ|Rohdaten");
  LCDSimul("n");
  LCDSimul(String(tmpWeight, 3));
  LCDSimul(" ");
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul(" ");
  LCDSimul(String(tmpValue,0)); 
  LCDSimul("n"); 
  LCDSimul("<Tara um zu nullen >");
  LCDSimul("n");
  /*Serial.print(scale.read());
  Serial.print("  ");
  Serial.print(scale.get_scale());
  Serial.print("  ");
  Serial.print(scale.get_offset());   
  Serial.print("  ");
  Serial.println(tmpWeight,3); */
}

void menu1(){
  if(menuOnce!=1){
    sensorTypeTemp=sensorType;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sensorwahl");
    lcd.setCursor(18,0);
    lcd.print("1");
    lcd.setCursor(0,1);
    lcd.print("Akt. Sensor: ");    
    lcd.setCursor(0,2);
    lcd.print("neuer Sensor:");
    lcd.setCursor(0,3);
    lcd.print("up/down/ok Auswahl");
    menuOnce=1;
  }
    lcd.setCursor(13,1);
    lcd.print(Sensor[sensorType]);
    lcd.setCursor(14,2);
    lcd.print(Sensor[sensorTypeTemp]);

    LCDSimul("Sensorwahl   1");
    LCDSimul("n");
    LCDSimul("Akt. Sensor: ");
    LCDSimul(String(Sensor[sensorType]));
    LCDSimul("n");
    LCDSimul("Neuer Sensor: ");
    LCDSimul(String(Sensor[sensorTypeTemp]));
    LCDSimul("n");
    LCDSimul("up/down/ok");
    LCDSimul("n");

}

void menu2(){
  if(menuOnce!=2){
    calGew=0;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Sensor kalibrieren");
    lcd.setCursor(18,0);
    lcd.print("2");
    lcd.setCursor(0,1);
    lcd.print(Sensor[sensorType]);
    lcd.setCursor(6,1);
    lcd.print("Sensor, Testgew.(kg)");
    lcd.setCursor(0,3);
    lcd.print("up/down/ok"); 
    menuOnce=2;
  }
  lcd.setCursor(0,2);
  lcd.print(tmpWeight,3);
  lcd.setCursor(7,2);
  lcd.print(calGew,3);
  lcd.setCursor(12,2);
  lcd.print(calibration_factor[sensorType]);

  LCDSimul("Sensor kalibrieren    2");
  LCDSimul("n");
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul(" Sensor, Testgewicht in kg");
  LCDSimul("n");
  LCDSimul(String(tmpWeight,3));
  LCDSimul(" ");
  LCDSimul(String(calGew,3));
  LCDSimul(" ");
  LCDSimul(String(calibration_factor[sensorType]));
  LCDSimul("n");
  LCDSimul("up/down/ok");
  LCDSimul("n");
}

void menu3(){
  if(menuOnce!=3){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Motorsteuerung");
    lcd.setCursor(18,0);
    lcd.print("3");
    lcd.setCursor(0,1);
    lcd.print("Kraft");
    lcd.setCursor(0,3);
    lcd.print("Up/down: Zug auf/ab");    
    menuOnce=3;
  }
  lcd.setCursor(2,0);
  lcd.print(tmpWeight,3);
  LCDSimul("Motorsteuerung    3");
  LCDSimul("n");
  LCDSimul("Richtung|Kraft");
  LCDSimul("n");
  LCDSimul(String(tmpWeight,3));
  LCDSimul("n");
  LCDSimul("Up/down:Zug auf/ab");
  LCDSimul("n");
}
  
void menu4(){
  if(menuOnce!=4){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Klebstoffprüfung");
    lcd.setCursor(18,0);
    lcd.print("4");
    lcd.setCursor(0,1);
    lcd.print("Kraft");
    lcd.setCursor(0,3);
    lcd.print("Ok - Test Starten");
    menuOnce=4;
  }
  lcd.setCursor(0,2);
  lcd.print(tmpWeight,3);
  lcd.setCursor(6,2);
  lcd.print(Sensor[sensorType]);
  LCDSimul("Klebstoffprüfung    4");
  LCDSimul("n");  
  LCDSimul("Kraft:");  
  LCDSimul("n");  
  LCDSimul(String(tmpWeight,3));
  LCDSimul("  ");
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul("n");  
  LCDSimul("OK - Test starten");
  LCDSimul("n");      
}

void motor(){//dirakt richtung des Motors; 1=zug aufbauen, -1=zug ablassen, 0=motor stoppen
   if(menuNr==0)
    dirakt=0;
   if(menuNr==1)
    dirakt=0;
   if(menuNr==2)
    dirakt=0;
   
   if(dirakt==0){
    /*Wire.beginTransmission(address);
    Wire.write(GPIOA); // port A adressieren
    Wire.write(0b000000000); 
    Wire.endTransmission();*/
    digitalWrite(motorL,LOW);
    digitalWrite(motorR,LOW);
   }
   if(dirakt==1){
    /*Wire.beginTransmission(address);
    Wire.write(GPIOA); // port A adressieren
    Wire.write(0b000000001); 
    Wire.endTransmission();*/
    digitalWrite(motorL,HIGH);
    digitalWrite(motorR,LOW);
    
   }
   if(dirakt==-1){
    /*Wire.beginTransmission(address);
    Wire.write(GPIOA); // port A adressieren
    Wire.write(0b000000010); 
    Wire.endTransmission();*/
    digitalWrite(motorL,LOW);
    digitalWrite(motorR,HIGH);
   }
}

void Belastungstest(){
  if(belastungstest==1){
    if(tmpWeight<1){
    dirakt=1;
    Debug("test1");
    }
    else{
    dirakt=0;
    Debug("Test2");
    belastungstest=0;
  }
  
  }
}

void SerialReceive(){//Empfängt, verarbeitet und verteilt eingehende Befehle von einem Seriellen anschluss
  if(SerialExchange){
    
  }
}

void SerialSend(){ //Sendet Seriell Informationen des Prüfstandes an den PC
  if(SerialExchange){
    Serial.print(tmpRaw);
    Serial.print(" ");
    Serial.print(tmpOffset);
    Serial.print(" ");
    Serial.print(calibration_factor[sensorType]);
    Serial.print(" ");
    Serial.print(tmpWeight);
    Serial.print(" ");
    Serial.print(menuNr);
    Serial.print(" ");
    Serial.print(sensorType);
    Serial.print(" ");
    Serial.print(dirakt);
    Serial.print(" ");
    Serial.print("");
    Serial.print(" ");
    Serial.println("");
  }
}

byte shifting(byte input, int isol){  //isol: zu isolierender Bit
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

