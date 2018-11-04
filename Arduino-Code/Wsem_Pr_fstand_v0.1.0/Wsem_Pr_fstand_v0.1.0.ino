//neue motoransteuerung
/*
   Software zur Verwendung mit einem Klebstoff-Prüfstand im Rahmen der Seminararbeit in der Oberstufe des Gymnasiums 2018
   Autor: Paul Sänger


   Anschlussanweisungen:
     -HX711 Modul:
       -Wägezelle zu HX711:
          Rot     -   E+
          Schwarz -   E-
          Grün    -   A-
          Weiß    -   A+
       -Modul zu Arduino:
          GND     -   GND
          VCC     -   5V
          DT      -   Definierter Arduino Pin DT (6)
          SCK     -   Definierter Arduino Pin SCK(7)
     -Bedienelement:
       -MCP23017 zu Arduino:
          Pin 9 |VDD    -   5V
          Pin 18|RESET  -   5V
          Pin 10|VSS    -   GND
          Pin 12|SCL    -   A5|SCL
          Pin 13|SDA    -   A4|SDA
          Pin 15|A0     -   GND
          Pin 16|A1     -   GND
          Pin 17|A2     -   GND
          I²C-Addresse über Pin 15 - Pin 17 am MCP23017 durch verbinden mit GND oder 5V, alle 3 zu GND: Adresse 0x20/100000; Adresse 100[angeschlossene leitungen] bsp: alle zu 5V: 100[111]
       -Taster zu MCP23017:
          Über Pulldown-Wiederstände geschaltet: 5V an ein Buttonterminal, Verbindung zu MCP23017 an anderes Terminal + Verbindung zu  GND über hochohmigen Wiederstand.
          OK    -   GPB0|Pin 1
          UP    -   GBP1|Pin 2
          DOWN  -   GBP2|Pin 3
          LEFT  -   GBP3|Pin 4
          RIGHT -   GBP4|Pin 5
          TARE  -   GBP5|Pin 6
     -Folientastatur:
       -MCP23017 zu Arduino:
          Pin 9 |VDD    -   5V
          Pin 18|RESET  -   5V
          Pin 10|VSS    -   GND
          Pin 12|SCL    -   A5|SCL
          Pin 13|SDA    -   A4|SDA
          Pin 15|A0     -   5V
          Pin 16|A1     -   GND
          Pin 17|A2     -   GND
          I²C-Addresse über Pin 15 - Pin 17 am MCP23017 durch verbinden mit GND oder 5V, alle 3 zu GND: Adresse 0x20/100000; Adresse 100[angeschlossene leitungen] bsp: alle zu 5V: 100[111]
       -Folientastatur zu MCP23017:
          Pin 1     -   Pin 21|GPA0
          Pin 2     -   Pin 22|GPA1
          Pin 3     -   Pin 23|GPA2
          Pin 4     -   Pin 24|GPA3
          Pin 5     -   Pin 1 |GPB0
          Pin 6     -   Pin 2 |GPB1
          Pin 7     -   Pin 3 |GPB2
          Pin 8     -   Pin 4 |GPB3
     -Anmeldung:
       -Schlüsselschalter zu Arduino:
          Pin 1     -   Über Pulldown-Wiederstand zu GND
          Pin 1     -   2
          Pin 2     -   5V
       -Offline Schalter zu Arduino:
          Pin 1     -   Über Pulldown-Wiederstand zu GND
          Pin 1     -   3
          Pin 2     -   5V
*/
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <Wire.h>
#define DT 6
#define SCK 7
HX711 scale (DT, SCK);
byte inputs = 0; // Inputs des bedienfeldes 
const int z = 4; //anzahl an Zeilen der Folientastatur
byte inputsFolie[z]; // Inputs der Folientastatur, auf zeilen aufgeteilt
const byte address = 0x20; //I²C-Adresse des portexpanders für Bedienelement
const byte adresse = 0x21; //I²C-Adresse des portexpanders für Folientastatur
const byte adresse2 = 0x22;//I²C-Adresse der Motorsteuerung
const byte  GPIOA = 0x12; //mcp23017 registeradresse
const byte  GPIOB = 0x13; //mcp23017 registeradresse
byte motorStat=0b00000000; //Status des Motordrivers
boolean blinkStat=true;
boolean blinkStat2=true;
boolean blinkStat3=true;
byte k=0; //Output für Bedienfeld
byte t=0; //Output für Folientastatur
const int ops = 9; //anzahl an previousMillis Variablen
unsigned long previousMillis[ops] = {};  //Speicher für Zeiteinheiten von zeitgesteuerten Event (z.B.: langes drücken einer Taste)
unsigned long currentMillis; //akuelle Millis (zeiteinheit)
int doonce[20];    //Arbeitsvariable
int doonceUp = 0; //nötig um langes Halten einer Taste zu registieren
int doonceDown = 0; //nötig um langes Halten einer Taste zu registieren
int doonceOk = 0; //nötig um langes Halten einer Taste zu registieren
int onceOK=0;
const int longPress = 600; //Intervall in millisekunden, ab dem ein Tastendruck als lang gilt
int dirakt = 0; //aktuelle Richtung des Motors
int belastungstest = 0; //status des belastungstest
float maxiBelastung=0; //Maximal erreichte belastung in KG
float maxiBelastungLast=0; //vorherige Maximal erreichte belastung in KG
const float minGew=0.100; //mindeste belastung die ein klebstoff aushalten muss 
int grund=0; //grund für belastungstest beendung
const String beenden[4]={
  {"       "},
  {"abbruch"},
  {"ende   "},
  {"user   "}
};

bool online = false; //speichert, ob mit PC verbunden


//input pins; Buttonmanagement
const int enable=2;//Schlüsselschalter,der den prüfstand sperren kann
int enableStart; //wert des schalters beim staren des Prüfstandes
const int jumper=3; //Button überspringt anmeldung am pc
const int dip1=10; //dipschalter, schaltet datenübertradung ein und aus
const int dip2=9;  //dipschalter, schaltet LCD simulation
const int dip3=8;//dipschalter, schaltet debugmodus
String Klebstoffpruefung("Klebstoffpruefung");

int upButtonCurrent = 0;
int upButtonLast = 0;

int downButtonCurrent = 0;
int downButtonLast = 0;

int leftButtonCurrent = 0;
int leftButtonLast = 0;

int rightButtonCurrent = 0;
int rightButtonLast = 0;

int okButtonCurrent = 0;
int okButtonLast = 0;

int tareButtonCurrent = 0;
int tareButtonLast = 0;

int recoverButtonCurrent = 0;
int recoverButtonLast = 0;

int Folie1 = 0;
int Folie1Last = 0;
int Folie2 = 0;
int Folie2Last = 0;
int Folie3 = 0;
int Folie3Last = 0;
int Folie4 = 0;
int Folie4Last = 0;
int Folie5 = 0;
int Folie5Last = 0;
int Folie6 = 0;
int Folie6Last = 0;
int Folie7 = 0;
int Folie7Last = 0;
int Folie8 = 0;
int Folie8Last = 0;
int Folie9 = 0;
int Folie9Last = 0;
int Folie0 = 0;
int Folie0Last = 0;
int FolieStern = 0;
int FolieSternLast = 0;
int FolieRaute = 0;
int FolieRauteLast = 0;
int FolieA = 0;
int FolieALast = 0;
int FolieB = 0;
int FolieBLast = 0;
int FolieC = 0;
int FolieCLast = 0;
int FolieD = 0;
int FolieDLast = 0;


const int motor1 = A3;
const int motor2 = A2;
const int mosfet=3;
const int motorEnable = 5;
int zustand=0;
int thresh=100;
int ramp=5;
int duty=0;


bool debug = false;
bool LCDSimulation = false;
bool SerialExchange = false;

const int menuMax = 5; //anzahl an untermenüs
int menuNr = 0; //aktuelles untermenü
int TestPhase = 0; //aktuelle phase des belastungstest
int menuOnce = -1; //verhindert unnötiges neuzeichnen von Menümasken

//Scale
float tmpWeight; //Speichert die Last auf der Zelle in kg für einen Programmzyklus für höhere Performanz des Codes
float tmpValue;  //Speichert die Rohdaten-Tarafaktor der Zelle für einen Programmzyklus für höhere Performanz des Codes
float tmpRaw;    //Speichert die Rohdaten der Zelle für einen Programmzyklus für höhere Performanz des Codes
float tmpOffset;  // ---"---
const int sens = 6; //Anzahl an eigespeicherten Sensoren
int sensorType = 0; //aktuell verwendeter Sensor
int sensorTypeTemp = 0; //zwischenspeicher bei Sensorwechsel
float calGew = 0; //Kalibrierungsgewicht
const float calGewMax = 9999.999; //Maximal erlaubter wert für calGew
float calibration_factor[sens] =  { 405959, //Kalibrierungsfaktor 5kg-Sensor
                                    -82979,//40596,    //Kalibrierungsfaktor 50kg-Sensor
                                    0,        //Kalibrierungsfaktor 200kg-Sensor
                                    0
                                  };       //Kalibrierungsfaktor weiterer Sensor
int maxBelastung[sens] = {49,    //maximale belastung eines sensors; schutz der Sensoren vor überlastung
                            490,
                            1962,
                            0,
                            0};
const String Sensor[sens] = {{"5KG  "},            //entsprechender sensorname
  {"50KG "},
  {"200KG"},
  {"usr01"},
  {"usr02"},
  {"usr03"}
};
  bool onc=true;


//Saveanimation
  byte arrow[3][8]={
    {
      B00100,
      B00100,
      B00100,
      B11111,
      B01110,
      B00100,
      B00000,
      B00000      
    },
    {
      B00000,
      B00100,
      B00100,
      B00100,
      B11111,
      B01110,
      B00100,   
      B00000   
    },
    {
      B00000,
      B00000,
      B00100,
      B00100,
      B00100,
      B11111,
      B01110,
      B00100   
    }
  };
int arrowDelay=200;//delay zwischen den einzelnen animationsframes
int arrowState=0; //Status der animation

//EEPROM
#define disk1 0x50    //Address of 24LC256 eeprom chip
int eepaddress=0;
int currentCalSave[6]; //Aktuell gespeicherte kalibrierungsfaktoren, verhindert unnötiges lesen aus eeprom


//LCD
LiquidCrystal_I2C lcd(0x27, 20, 4); //addr., spalten, zeilen des Displays


void setup() {
  Wire.setClock(10000);
  pinMode(dip1,INPUT);
  pinMode(dip2,INPUT);
  pinMode(dip3,INPUT);
  pinMode(mosfet,OUTPUT);
  analogWrite(mosfet,0);
  if(digitalRead(dip1)==HIGH) SerialExchange=true;
  else SerialExchange=false;
  if(digitalRead(dip2)==HIGH) LCDSimulation=true;
  else LCDSimulation=false;
  if(digitalRead(dip3)==HIGH) debug=true;
  else debug=false;
  Serial.begin(19200);// Übertragungsgeschwindigkeit an den PC (Standard: 9600; Schnellstes: 2000000)
  lcd.init();
  lcd.backlight();
  pinMode(enable,INPUT);
  pinMode(jumper,INPUT);
  enableStart=digitalRead(enable);
  while((digitalRead(jumper)==LOW)&&!Serial){
    if(onc){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warte auf Verbindung");
    delay(10);
    onc=false;}
  }
  if(Serial){
    Serial.println("connected");
    online=true;
    Serial.read();
  }
  if(digitalRead(jumper)==HIGH){
    Serial.println("connected");
    online=false;
  }
  onc=true;
  /*while((digitalRead(enable)==enableStart)){
    if(onc){
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Warte auf Freigabe");
    delay(10);
    onc=false;
    }
  }*/
  onc=true;
  delay(250);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Booting...");
  LCDSimul("Booting...");
  LCDSimul("n");
  delay(250);
  Wire.begin();
  LCDSimul("LCD initialising");
  LCDSimul("n");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LCD initialising");
  LCDSimul("set scale");
  LCDSimul("n");
  delay(250);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("set scale");
  scale.set_scale();
  LCDSimul("scale tare");
  LCDSimul("n");
  delay(250);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("scale tare");
  scale.tare();
  delay(250);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("loading");
  LCDSimul("loading");
  LCDSimul("n");
  delay(250);
   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("motor define");
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  pinMode(motorEnable, OUTPUT);
  digitalWrite(motor1, LOW);
  digitalWrite(motor2, LOW);
  pinMode(motorEnable, LOW);
   lcd.clear();
  lcd.setCursor(0, 0);
  
  lcd.print("UI initialise");
  Wire.beginTransmission(adresse);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // set all of port A to outputs
  Wire.endTransmission();
  
  Wire.beginTransmission(address);
  Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // set all of port A to outputs
  Wire.endTransmission();
  
   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("motor initialise");
  Wire.beginTransmission(adresse2);
   Wire.write(0x00); // IODIRA register
  Wire.write(0x00); // set all of port A to outputs
  Wire.endTransmission();
   lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("read eeprom");
  sensorType=readEEPROM(disk1, 0);
  if(readFloatEEPROM(disk1,1)!=0)calibration_factor[0]=readFloatEEPROM(disk1,1);
  if(readFloatEEPROM(disk1,5)!=0)calibration_factor[1]=readFloatEEPROM(disk1,5);
  if(readFloatEEPROM(disk1,9)!=0)calibration_factor[2]=readFloatEEPROM(disk1,9);
  if(readFloatEEPROM(disk1,13)!=0)calibration_factor[3]=readFloatEEPROM(disk1,13);
  if(readFloatEEPROM(disk1,17)!=0)calibration_factor[4]=readFloatEEPROM(disk1,17);
  if(readFloatEEPROM(disk1,21)!=0)calibration_factor[5]=readFloatEEPROM(disk1,21);
  for(int i=0;i<3;i++){
    lcd.createChar(i, arrow[i]);
  }
  LCDSimul("Ready!");
  LCDSimul("n");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ready");
  delay(500);
  lcd.clear();

}

void loop() {
  if (onc!=true)onc=true;
  /*while(digitalRead(enable)==LOW){
    if(onc){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gesperrt");
      menuOnce=-1;
      dirakt=0;
      onc=false;
    }
  }*/
  scale.set_scale(calibration_factor[sensorType]);
  tmpRaw = scale.read_average(1);
  tmpOffset = scale.get_offset();
  tmpValue = tmpRaw - tmpOffset;
  tmpWeight = tmpValue / scale.get_scale();
  Debug();
  SerialReceive();
  ButtonEvent();
  showMenu();
  Belastungstest();
  motor();
  SerialSend();
  saveAni();
  currentMillis = millis();
  //Serial.print("                                              ");
  //Serial.println(currentMillis - previousMillis[0]);
  previousMillis[0] = currentMillis;
  motorBlink();
  delay(1);
}

void ButtonEvent() { //Aufschlüsseln des GPIOB Inputs, vorbereiten, managen und zur Verfügung stellen der Button Inputs
  Wire.beginTransmission(address);
  Wire.write(GPIOB);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  inputs = Wire.read();
  Debug(String(inputs));
  //Byteshifting um die einzelnen zustände der Buttons aus dem gesammten Byte zu isolieren und ihren variablen zuzuordnen
  byte temp;
  temp = shifting(inputs, 1);
  if (temp == 1)
    okButtonCurrent = 1;
  else if (temp == 0)
    okButtonCurrent = 0;
  temp = shifting(inputs, 2);
  if (temp == 1)
    upButtonCurrent = 1;
  else if (temp == 0)
    upButtonCurrent = 0;
  temp = shifting(inputs, 3);
  if (temp == 1)
    downButtonCurrent = 1;
  else if (temp == 0)
    downButtonCurrent = 0;
  temp = shifting(inputs, 4);
  if (temp == 1)
    leftButtonCurrent = 1;
  else if (temp == 0)
    leftButtonCurrent = 0;
  temp = shifting(inputs, 5);
  if (temp == 1)
    rightButtonCurrent = 1;
  else if (temp == 0)
    rightButtonCurrent = 0;
  temp = shifting(inputs, 6);
  if (temp == 1)
    tareButtonCurrent = 1;
  else if (temp == 0)
    tareButtonCurrent = 0;
  temp = shifting(inputs, 7);
  if (temp == 1)
    recoverButtonCurrent = 1;
  else if (temp == 0)
    recoverButtonCurrent = 0;
  for(int i=0;i<z;i++){
    
    if(i==0){
      bitWrite(t,0,1);
      bitWrite(t,1,0);
      bitWrite(t,2,0);
      bitWrite(t,3,0);
      };
    if(i==1){
      bitWrite(t,0,0);
      bitWrite(t,1,1);
      bitWrite(t,2,0);
      bitWrite(t,3,0);
      };
    if(i==2){
      bitWrite(t,0,0);
      bitWrite(t,1,0);
      bitWrite(t,2,1);
      bitWrite(t,3,0);
      };
    if(i==3){
      bitWrite(t,0,0);
      bitWrite(t,1,0);
      bitWrite(t,2,0);
      bitWrite(t,3,1);
      };
    
    if(currentMillis-previousMillis[7]>350){
      if(blinkStat2==true){
        bitWrite(t, 4, 1);
        //motorStat=motorStat+16;
        blinkStat2=false;
      }
      else if(blinkStat2==false){
        bitWrite(t, 4, 0);
        //motorStat=motorStat-16;
        blinkStat2=true;
      }
      previousMillis[7]=currentMillis;
    }
    Wire.beginTransmission(adresse);
    Wire.write(GPIOA);
    Wire.write(t);
    Wire.endTransmission();
    
    Wire.beginTransmission(adresse);
    Wire.write(GPIOB);
    Wire.endTransmission();
    Wire.requestFrom(adresse, 1);
    inputsFolie[i] = Wire.read();
    Debug(String(inputsFolie[i]));
  }

    //Byteshifting um die einzelnen zustände der Buttons aus dem gesammten Byte zu isolieren und ihren variablen zuzuordnen
  temp = shifting(inputsFolie[0],1);
  if (temp==1)
    Folie1 = 1;
  else if (temp==0)
    Folie1 = 0;
  temp = shifting(inputsFolie[0],2);
  if (temp==1)
    Folie2 = 1;
  else if (temp==0)
    Folie2 = 0;
  temp = shifting(inputsFolie[0],3);
  if (temp==1)
    Folie3 = 1;
  else if (temp==0)
    Folie3 = 0;
  temp = shifting(inputsFolie[0],4);
  if (temp==1)
    FolieA = 1;
  else if (temp==0)
    FolieA = 0;
  temp = shifting(inputsFolie[1],1);
  if (temp==1)
    Folie4 = 1;
  else if (temp==0)
    Folie4 = 0;
  temp = shifting(inputsFolie[1],2);
  if (temp==1)
    Folie5 = 1;
  else if (temp==0)
    Folie5 = 0;
  temp = shifting(inputsFolie[1],3);
  if (temp==1)
    Folie6 = 1;
  else if (temp==0)
    Folie6 = 0;
  temp = shifting(inputsFolie[1],4);
  if (temp==1)
    FolieB = 1;
  else if (temp==0)
    FolieB = 0;
  temp = shifting(inputsFolie[2],1);
  if (temp==1)
    Folie7 = 1;
  else if (temp==0)
    Folie7 = 0;
  temp = shifting(inputsFolie[2],2);
  if (temp==1)
    Folie8 = 1;
  else if (temp==0)
    Folie8 = 0;
  temp = shifting(inputsFolie[2],3);
  if (temp==1)
    Folie9 = 1;
  else if (temp==0)
    Folie9 = 0;
  temp = shifting(inputsFolie[2],4);
  if (temp==1)
    FolieC = 1;
  else if (temp==0)
    FolieC = 0;
  temp = shifting(inputsFolie[3],1);
  if (temp==1)
    FolieStern = 1;
  else if (temp==0)
    FolieStern = 0;
  temp = shifting(inputsFolie[3],2);
  if (temp==1)
    Folie0 = 1;
  else if (temp==0)
    Folie0 = 0;
  temp = shifting(inputsFolie[3],3);
  if (temp==1)
    FolieRaute = 1;
  else if (temp==0)
    FolieRaute = 0;
  temp = shifting(inputsFolie[3],4);
  if (temp==1)
    FolieD = 1;
  else if (temp==0)
    FolieD = 0;
  



  if (tareButtonCurrent == 1) {
    tareButtonLast = 1;
    if (doonce[0] == 0) {
      buttonEventTare();//tareButton Event
      doonce[0] = 1;
    }
  }
  if (tareButtonCurrent == 0) {
    if (tareButtonLast == 1) {
      //TareButtonRelease Event, wird getriggert, wenn der Taster losgelassen wird
      tareButtonLast = 0;
    }
    doonce[0] = 0; //zurücksetzen des nötigen do once, um ein erneutes aufrufen des buttonEvents zu ermöglichen (debouncing durch trägheit des codes)
  }

  if (okButtonCurrent == 1) {
    okButtonLast = 1;
    if (doonce[1] == 0) {
      buttonEventOk();//OkButton Event, wird direkt bei drücken der taste ausgelößt 
      doonce[1] = 1;
    }
    if (doonceOk == 0) {
      previousMillis[3] = currentMillis; //Zeitmessung starten
      doonceOk = 1;
    }

  }
  if (okButtonCurrent == 0) {
    if (okButtonLast == 1) {
      if (currentMillis - previousMillis[3] >= longPress) {
        buttonEventOkLong();      //Event getriggert nach langem halten und loslassen
      }
      else {
        buttonEventOkShort();     //Event getriggert nach kurzem halten und loslassen
      }
      okButtonLast = 0;
      doonceOk = 0;
    }
    doonce[1] = 0;
  }

  if (rightButtonCurrent == 1) {
    rightButtonLast = 1;
  }
  if (rightButtonCurrent == 0) {
    if (rightButtonLast == 1) {
      buttonEventRight();//rightButton Event
      rightButtonLast = 0;
    }
  }

  if (leftButtonCurrent == 1) {
    leftButtonLast = 1;
  }
  if (leftButtonCurrent == 0) {
    if (leftButtonLast == 1) {
      buttonEventLeft();//leftButton Event
      leftButtonLast = 0;
    }
  }

  if (upButtonCurrent == 1) {
    upButtonLast = 1;
    if (doonce[2] == 0) {
      buttonEventUp();
      doonce[2] = 1;
    }
    if (doonceUp == 0) {
      previousMillis[1] = currentMillis;
      doonceUp = 1;
    }
  }
  if (upButtonCurrent == 0) {
    if (upButtonLast == 1) {
      if (currentMillis - previousMillis[1] >= longPress) {
        buttonEventUpLong();
      }
      else {
        buttonEventUpShort();
      }
      upButtonLast = 0;
      doonceUp = 0;
    }
    doonce[2] = 0;
  }

  if (downButtonCurrent == 1) {
    downButtonLast = 1;
    if (doonce[3] == 0) {
      buttonEventDown();
      doonce[3] = 1;
    }
    if (doonceDown == 0) {
      previousMillis[2] = currentMillis;
      doonceDown = 1;
    }
  }
  if (downButtonCurrent == 0) {
    if (downButtonLast == 1) {
      if (currentMillis - previousMillis[2] >= longPress) {
        buttonEventDownLong();
      }
      else {
        buttonEventDownShort();
      }
      downButtonLast = 0;
      doonceDown = 0;
    }
    doonce[3] = 0;
  }

  if (recoverButtonCurrent == 1) {
    recoverButtonLast = 1;
    if (doonce[4] == 0) {
      buttonEventRecover();
      doonce[4] = 1;
      previousMillis[3] = currentMillis;
    }
  }
  if (recoverButtonCurrent == 0) {
    if (recoverButtonLast == 1) {
      if (currentMillis - previousMillis[3] >= longPress) {
        buttonEventRecoverLong();
      }
      else {
        buttonEventRecoverShort();
      }
      recoverButtonLast = 0;
    }
    doonce[4] = 0;
  }

  if (Folie1 == 1) {
    Folie1Last = 1;
    if (doonce[5] == 0) {
      calGew=(calGew*10)+0.001;
      doonce[5] = 1;
    }
  }
  if (Folie1 == 0) {
    if (Folie1Last == 1) {
      Folie1Last = 0;
    }
    doonce[5] = 0; 
  }

  if (Folie2 == 1) {
    Folie2Last = 1;
    if (doonce[6] == 0) {
      calGew=(calGew*10)+0.002;
      doonce[6] = 1;
    }
  }
  if (Folie2 == 0) {
    if (Folie2Last == 1) {
      Folie2Last = 0;
    }
    doonce[6] = 0; 
  }
  
  if (Folie3 == 1) {
    Folie3Last = 1;
    if (doonce[7] == 0) {
      calGew=(calGew*10)+0.003;
      doonce[7] = 1;
    }
  }
  if (Folie3 == 0) {
    if (Folie3Last == 1) {
      Folie3Last = 0;
    }
    doonce[7] = 0; 
  }
  
  if (Folie4 == 1) {
    Folie4Last = 1;
    if (doonce[8] == 0) {
      calGew=(calGew*10)+0.004;
      doonce[8] = 1;
    }
  }
  if (Folie4 == 0) {
    if (Folie4Last == 1) {
      Folie4Last = 0;
    }
    doonce[8] = 0; 
  }
  
  if (Folie5 == 1) {
    Folie5Last = 1;
    if (doonce[9] == 0) {
      calGew=(calGew*10)+0.005;
      doonce[9] = 1;
    }
  }
  if (Folie5 == 0) {
    if (Folie5Last == 1) {
      Folie5Last = 0;
    }
    doonce[9] = 0; 
  }
  
  if (Folie6 == 1) {
    Folie6Last = 1;
    if (doonce[10] == 0) {
      calGew=(calGew*10)+0.006;
      doonce[10] = 1;
    }
  }
  if (Folie6 == 0) {
    if (Folie6Last == 1) {
      Folie6Last = 0;
    }
    doonce[10] = 0; 
  }
  
  if (Folie7 == 1) {
    Folie7Last = 1;
    if (doonce[11] == 0) {
      calGew=(calGew*10)+0.007;
      doonce[11] = 1;
    }
  }
  if (Folie7 == 0) {
    if (Folie7Last == 1) {
      Folie7Last = 0;
    }
    doonce[11] = 0; 
  }
  
  if (Folie8 == 1) {
    Folie8Last = 1;
    if (doonce[12] == 0) {
      calGew=(calGew*10)+0.008;
      doonce[12] = 1;
    }
  }
  if (Folie8 == 0) {
    if (Folie8Last == 1) {
      Folie8Last = 0;
    }
    doonce[12] = 0; 
  }
  
  if (Folie9 == 1) {
    Folie9Last = 1;
    if (doonce[13] == 0) {
      calGew=(calGew*10)+0.009;
      doonce[13] = 1;
    }
  }
  if (Folie9 == 0) {
    if (Folie9Last == 1) {
      Folie9Last = 0;
    }
    doonce[13] = 0; 
  }
  
  if (Folie0 == 1) {
    Folie0Last = 1;
    if (doonce[14] == 0) {
      calGew=(calGew*10)+0.000;
      doonce[14] = 1;
    }
  }
  if (Folie0 == 0) {
    if (Folie0Last == 1) {
      Folie0Last = 0;
    }
    doonce[14] = 0; 
  }
  
  if (FolieA == 1) {
    FolieALast = 1;
    if (doonce[15] == 0) {
      calGew=0;
      if(calGew<0.001)
        calGew=0;
      doonce[15] = 1;
      menuOnce = -1;
    }
  }
  if (FolieA == 0) {
    if (FolieALast == 1) {
      FolieALast = 0;
    }
    doonce[15] = 0; 
  }
  
  if (FolieStern == 1) {
    FolieSternLast = 1;
    if (doonce[16] == 0) {
      buttonEventLeft();
      doonce[16] = 1;
    }
  }
  if (FolieStern == 0) {
    if (FolieSternLast == 1) {
      FolieSternLast = 0;
    }
    doonce[16] = 0; 
  }
  
  if (FolieRaute == 1) {
    FolieRauteLast = 1;
    if (doonce[17] == 0) {
      buttonEventRight();
      doonce[17] = 1;
    }
  }
  if (FolieRaute == 0) {
    if (FolieRauteLast == 1) {
      FolieRauteLast = 0;
    }
    doonce[17] = 0; 
  }




}

void buttonEventOk() {
  switch (menuNr) {
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
void buttonEventOkShort() {
  switch (menuNr) {
    case 0:
      menu0EventOkShort();
      break;
    case 1:
      menu1EventOkShort();
      break;
    case 2:
      menu2EventOkShort();
      break;
    case 3:
      menu3EventOkShort();
      break;
    case 4:
      menu4EventOkShort();
      break;
  }
}
void buttonEventOkLong() {
  switch (menuNr) {
    case 0:
      menu0EventOkLong();
      break;
    case 1:
      menu1EventOkLong();
      break;
    case 2:
      menu2EventOkLong();
      break;
    case 3:
      menu3EventOkLong();
      break;
    case 4:
      menu4EventOkLong();
      break;
  }
}

void buttonEventUp() {
  switch (menuNr) {
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
void buttonEventUpShort() {
  switch (menuNr) {
    case 0:
      menu0EventUpShort();
      break;
    case 1:
      menu1EventUpShort();
      break;
    case 2:
      menu2EventUpShort();
      break;
    case 3:
      menu3EventUpShort();
      break;
    case 4:
      menu4EventUpShort();
      break;
  }
}
void buttonEventUpLong() {
  switch (menuNr) {
    case 0:
      menu0EventUpLong();
      break;
    case 1:
      menu1EventUpLong();
      break;
    case 2:
      menu2EventUpLong();
      break;
    case 3:
      menu3EventUpLong();
      break;
    case 4:
      menu4EventUpLong();
      break;
  }
}

void buttonEventDown() {
  switch (menuNr) {
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
void buttonEventDownShort() {
  switch (menuNr) {
    case 0:
      menu0EventDownShort();
      break;
    case 1:
      menu1EventDownShort();
      break;
    case 2:
      menu2EventDownShort();
      break;
    case 3:
      menu3EventDownShort();
      break;
    case 4:
      menu4EventDownShort();
      break;
  }
}
void buttonEventDownLong() {
  switch (menuNr) {
    case 0:
      menu0EventDownLong();
      break;
    case 1:
      menu1EventDownLong();
      break;
    case 2:
      menu2EventDownLong();
      break;
    case 3:
      menu3EventDownLong();
      break;
    case 4:
      menu4EventDownLong();
      break;
  }
}

void buttonEventLeft() {
  if(belastungstest==0){
    menuMinus();
  }
  else{
    BelastungstestAbbruch();
  }
}

void buttonEventRight() {
  if(belastungstest==0){
    menuPlus();
  }
  else{
    BelastungstestAbbruch();
  }
}

void buttonEventTare() {
  switch (menuNr) {
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
}
void buttonEventRecover(){
  
}
void buttonEventRecoverLong(){
  
}
void buttonEventRecoverShort(){
  
}


void menu0EventOk() {

}
void menu0EventOkShort() {

}
void menu0EventOkLong() {

}

void menu1EventOk() {
  sensorType = sensorTypeTemp;
}
void menu1EventOkShort() {
  sensorType = sensorTypeTemp;
}
void menu1EventOkLong() {
  writeEEPROM(disk1, 0,sensorType);
  Debug("EEPROM geschrieben");
  save();
}

void menu2EventOk() {

}
void menu2EventOkShort() {
  calibration_factor[sensorType] = scale.get_value(10) / calGew;
}
void menu2EventOkLong() {
  switch(sensorType){
    case 0:
      writeFloatEEPROM(disk1, 1, calibration_factor[0]);
    break;
    case 1:
      writeFloatEEPROM(disk1, 5, calibration_factor[1]);
    break;
    case 2:
      writeFloatEEPROM(disk1, 9, calibration_factor[2]);
    break;
    case 3:
      writeFloatEEPROM(disk1, 13, calibration_factor[3]);
    break;
    case 4:
      writeFloatEEPROM(disk1, 17, calibration_factor[4]);
    break;
    case 5:
      writeFloatEEPROM(disk1, 21, calibration_factor[5]);
    break;
    default:
      
    break;
  }
  save();
  //EEPROM speichern
}

void menu3EventOk() {
  if(tmpWeight>(10*minGew)||tmpWeight<(-10*minGew)){
    if(tmpWeight>minGew){
      dirakt=-1;
    }
    if(tmpWeight<minGew){
      dirakt=1;
    }
  }
  else{
    dirakt = 0;
  }
}
void menu3EventOkShort() {
  dirakt = 0;
}
void menu3EventOkLong() {
  dirakt = 0;
}

void menu4EventOk() {
  //belastungstest = 1;
  switch(belastungstest){
    case 0:
      belastungstest=1;
      break;
    case 8:
      belastungstest=0;
      dirakt=0;
      break;
    case 9:
      belastungstest=0;
      dirakt=0;
      break;
    default:
      BelastungstestAbbruch();
      break;
  }
}
void menu4EventOkShort() {

}
void menu4EventOkLong() {

}

void menu0EventUp() {

}
void menu0EventUpShort() {

}
void menu0EventUpLong() {

}

void menu1EventUp() {
  sensorTypeTemp++;
  if (sensorTypeTemp >= sens) {
    sensorTypeTemp = 0;
  }
}
void menu1EventUpShort() {

}
void menu1EventUpLong() {

}

void menu2EventUp() {

}
void menu2EventUpShort() {
  calGew = calGew + 0.0010;
}
void menu2EventUpLong() {
  calGew = calGew + 0.1;
}

void menu3EventUp() {
  dirakt = 1;
}
void menu3EventUpShort() {
  dirakt = 0;
}
void menu3EventUpLong() {
  dirakt = 0;
}

void menu4EventUp() {

}
void menu4EventUpShort() {

}
void menu4EventUpLong() {

}

void menu0EventDown() {

}
void menu0EventDownShort() {

}
void menu0EventDownLong() {

}

void menu1EventDown() {
  sensorTypeTemp--;
  if (sensorTypeTemp <= -1) {
    sensorTypeTemp = sens - 1;
  }
}
void menu1EventDownShort() {

}
void menu1EventDownLong() {

}

void menu2EventDown() {

}
void menu2EventDownShort() {
  calGew = calGew - 0.0010;
  if (calGew < 0) {
    calGew = 0;
  }
}
void menu2EventDownLong() {
  calGew = calGew - 0.1;
  if (calGew < 0) {
    calGew = 0;
  }
}

void menu3EventDown() {
  dirakt = -1;
}
void menu3EventDownShort() {
  dirakt = 0;
}
void menu3EventDownLong() {
  dirakt = 0;
}

void menu4EventDown() {

}
void menu4EventDownShort() {

}
void menu4EventDownLong() {

}

void menu0EventTare() {
  scale.tare(10);
}
void menu1EventTare() {
  scale.tare(10);
}
void menu2EventTare() {
  scale.tare(10);
}
void menu3EventTare() {
  scale.tare(10);
}
void menu4EventTare() {
  scale.tare(10);
}

void menuPlus() {
  menuNr++;
  if (menuNr >= menuMax)
    menuNr = 0;
}
void menuMinus() {
  menuNr--;
  if (menuNr < 0)
    menuNr = menuMax - 1;
}

void showMenu() {
  switch (menuNr) {
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

void menu0() {
  if (menuOnce != 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Waage");
    lcd.setCursor(19, 0);
    lcd.print("0");
    lcd.setCursor(0, 1);
    lcd.print("Gewicht:");
    lcd.setCursor(0,2);
    lcd.print("Rohdaten:");
    lcd.setCursor(0,3);
    lcd.print("Typ:");
    lcd.setCursor(5, 3);
    lcd.print(Sensor[sensorType]);
    menuOnce = 0;
  }
  lcd.setCursor(9, 1);
  lcd.print(tmpWeight, 3);
  for(int i=0;i<(9-(((String)tmpWeight).length()));i++){
    lcd.print(" ");
  }
  lcd.setCursor(10, 2);
  lcd.print(tmpValue,0);
  for(int i=0;i<(11-((String)tmpValue).length());i++){
    lcd.print(" ");
  }

  LCDSimul(F("Waage     0"));
  LCDSimul("n");
  LCDSimul(F("Gewicht|Typ|Rohdaten"));
  LCDSimul("n");
  LCDSimul(String(tmpWeight, 3));
  LCDSimul(" ");
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul(" ");
  LCDSimul(String(tmpValue, 0));
  LCDSimul("n");
  LCDSimul(F("<Tara um zu nullen >"));
  LCDSimul("n");
}

void menu1() {
  if (menuOnce != 1) {
    sensorTypeTemp = sensorType;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Sensorwahl"));
    lcd.setCursor(19, 0);
    lcd.print("1");
    lcd.setCursor(0, 1);
    lcd.print(F("Akt. Sensor: "));
    lcd.setCursor(0, 2);
    lcd.print(F("neuer Sensor:"));
    menuOnce = 1;
  }
  lcd.setCursor(14, 1);
  lcd.print(Sensor[sensorType]);
  lcd.setCursor(14, 2);
  lcd.print(Sensor[sensorTypeTemp]);

  LCDSimul(F("Sensorwahl   1"));
  LCDSimul("n");
  LCDSimul(F("Akt. Sensor: "));
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul("n");
  LCDSimul(F("Neuer Sensor: "));
  LCDSimul(String(Sensor[sensorTypeTemp]));
  LCDSimul("n");
  LCDSimul(F("up/down/ok"));
  LCDSimul("n");

}

void menu2() {
  if (menuOnce != 2) {
    calGew = 0;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Kalibrieren"));
    lcd.setCursor(19, 0);
    lcd.print("2");
    lcd.setCursor(12, 0);
    lcd.print(Sensor[sensorType]);
    lcd.setCursor(0, 1);
    lcd.print(F("Akt. Gew.:"));
    lcd.setCursor(0, 2);
    lcd.print(F("Kal. Gew.:"));
    lcd.setCursor(0, 3);
    lcd.print(F("Kal. Fakt:"));
    menuOnce = 2;
  }
  lcd.setCursor(10, 1);
  lcd.print(tmpWeight, 3);
  for(int i=0;i<(8-((String)tmpWeight).length());i++){
    lcd.print(" ");
  }
  lcd.setCursor(10, 2);
  if(calGew>calGewMax){
    calGew=calGew/10;
  }
  lcd.print(calGew, 3);
  for(int i=0;i<(9-((String)calGew).length());i++){
    lcd.print(" ");
  }
  lcd.setCursor(10, 3);
  lcd.print(/*(int)*/calibration_factor[sensorType],0);
  //for(int i=0;i<(10-((String)((int)calibration_factor[sensorType])).length());i++){
    lcd.print(" ");
  //}
  Serial.println((int)calibration_factor[sensorType]);

  LCDSimul(F("Sensor kalibrieren    2"));
  LCDSimul("n");
  LCDSimul(String(Sensor[sensorType]));
  LCDSimul(F(" Sensor, Testgewicht in kg"));
  LCDSimul("n");
  LCDSimul(String(tmpWeight, 3));
  LCDSimul(" ");
  LCDSimul(String(calGew, 3));
  LCDSimul(" ");
  LCDSimul(String(calibration_factor[sensorType]));
  LCDSimul("n");
  LCDSimul(F("up/down/ok"));
  LCDSimul("n");
}

void menu3() {
  if (menuOnce != 3) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Motorsteuerung"));
    lcd.setCursor(19, 0);
    lcd.print("3");
    lcd.setCursor(0, 1);
    lcd.print(F("Kraft"));
    lcd.setCursor(0, 2);
    lcd.print(F("Richtung"));
    lcd.setCursor(0, 3);
    lcd.print(F("Up/down: Zug auf/ab"));
    menuOnce = 3;
  }
  lcd.setCursor(6, 1);
  lcd.print(tmpWeight, 3);
  for(int i=0;i<(8-((String)tmpWeight).length());i++){
    lcd.print(" ");
  }
  lcd.setCursor(9, 2);
  lcd.print(dirakt);
  lcd.print("  ");
  LCDSimul(F("Motorsteuerung    3"));
  LCDSimul("n");
  LCDSimul(F("Richtung|Kraft"));
  LCDSimul("n");
  LCDSimul(String(tmpWeight, 3));
  LCDSimul("n");
  LCDSimul(F("Up/down:Zug auf/ab"));
  LCDSimul("n");
}

void menu4() {
  if(belastungstest==0){
    if (menuOnce != 4) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0, 1);
      lcd.print(F("Kraft:"));
      lcd.setCursor(0, 2);
      lcd.print(F("Sensor:"));
      lcd.setCursor(0, 3);
      lcd.print(F("Ok - Test starten"));
      menuOnce = 4;
    }
    lcd.setCursor(7, 1);
    lcd.print(tmpWeight, 3);
    for(int i=0;i<(8-((String)tmpWeight).length());i++){
      lcd.print(" ");
    }
    lcd.setCursor(8, 2);
    lcd.print(Sensor[sensorType]);
    LCDSimul("Klebstoffprüfung    4");
    LCDSimul("n");
    LCDSimul("Kraft:");
    LCDSimul("n");
    LCDSimul(String(tmpWeight, 3));
    LCDSimul("  ");
    LCDSimul(String(Sensor[sensorType]));
    LCDSimul("n");
    LCDSimul(F("OK - Test starten"));
    LCDSimul("n");
  }
  if(belastungstest==1){
    if(menuOnce !=5){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0, 2);
      lcd.print(F("Übertragung starten"));
      menuOnce=5;
    }
  }
  if(belastungstest==2){
    if(menuOnce !=6){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0, 2);
      lcd.print("Variablen vorbereiten");
      menuOnce=6;
    }
  }
  if(belastungstest==3){
    if(menuOnce !=7){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0, 2);
      lcd.print("Zug starten");
      menuOnce=7;
    }
  }
  if(belastungstest==4){
    if(menuOnce !=8){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0, 1);
      lcd.print("Sensor");
      lcd.setCursor(7, 1);
      lcd.print(Sensor[sensorType]);
      lcd.setCursor(0,2);
      lcd.print("max. bel:");
      lcd.setCursor(11, 2);
      lcd.print(maxBelastung[sensorType]);
      lcd.setCursor(0,3);
      lcd.print("Gewicht:");
      menuOnce=8;
    }
    lcd.setCursor(11, 3);
    lcd.print(tmpWeight,3);
    for(int i=0;i<(7-((String)tmpWeight).length());i++){
      lcd.print(" ");
    }
  }
  if(belastungstest==5){
    if(menuOnce !=9){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0,2);
      lcd.print("Überlastung");
      menuOnce=9;
    }
  }
  if(belastungstest==6){
    if(menuOnce !=10){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0,2);
      lcd.print("Ende");
      menuOnce=10;
    }
  } 
  if(belastungstest==7){
    if(menuOnce !=11){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0,2);
      lcd.print("User abbruch");
      menuOnce=11;
    }
  }
  if(belastungstest==8||belastungstest==9){
    if(menuOnce !=12){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(Klebstoffpruefung);
      lcd.setCursor(19, 0);
      lcd.print("4");
      lcd.setCursor(0, 1);
      lcd.print("Ende,Grund:");
      lcd.print("");
      lcd.setCursor(12, 1);
      lcd.print(beenden[grund]);
      lcd.setCursor(0, 2);
      lcd.print("max. bel:");
      lcd.print(maxiBelastung);
      lcd.setCursor(0,3);
      lcd.print("Sensor:");
      lcd.setCursor(8 ,3);
      lcd.print(Sensor[sensorType]);
      menuOnce=12;
    }
  }
}

void motorBlink(){
  if(currentMillis-previousMillis[5]>400){
    if(blinkStat==true){
      bitWrite(motorStat, 4, 1);
      //motorStat=motorStat+16;
      blinkStat=false;
    }
    else if(blinkStat==false){
      bitWrite(motorStat, 4, 0);
      //motorStat=motorStat-16;
      blinkStat=true;
    }
    previousMillis[5]=currentMillis;
  }
  
  if(currentMillis-previousMillis[8]>450){
    if(blinkStat3==true){
      bitWrite(k, 0, 1);
      //motorStat=motorStat+16;
      blinkStat3=false;
    }
    else if(blinkStat3==false){
      bitWrite(k, 0, 0);
      //motorStat=motorStat-16;
      blinkStat3=true;
    }
    previousMillis[8]=currentMillis;
  }
  Wire.beginTransmission(address);
  Wire.write(GPIOA);
  Wire.write(k);
  Wire.endTransmission();
  //Serial.println((byte)motorStat);
}

void motor() { //dirakt richtung des Motors; 1=zug aufbauen, -1=zug ablassen, 0=motor stoppen
  if (menuNr == 0)
    dirakt = 0;
  if (menuNr == 1)
    dirakt = 0;
  if (menuNr == 2)
    dirakt = 0;

  switch(zustand){
    case 0:
      bitWrite(motorStat, 0, 0);
      bitWrite(motorStat, 1, 0);
      bitWrite(motorStat, 2, 0);
      bitWrite(motorStat, 3, 0);
      
      switch(dirakt){
        case 0:
          zustand=0;
        break;
        case 1:
          zustand=1;
          previousMillis[6]=currentMillis;
        break;
        case -1:
          zustand=2;
          previousMillis[6]=currentMillis;
        break;
      }
    break;
    case 1:
      bitWrite(motorStat, 2, 0);
      bitWrite(motorStat, 3, 0);
      switch(dirakt){
        case 0:
          zustand=0;
        break;
        case 1:
          if(currentMillis-previousMillis[6]>=thresh){
            previousMillis[6]=currentMillis;
            zustand=3;
            duty=0;
            analogWrite(mosfet,duty);
          } 
        break;
        case -1:
        if(currentMillis-previousMillis[6]>=thresh){
         previousMillis[6]=currentMillis; 
          zustand=2;
        }
        break;
      }
    break;
    case 2:
      bitWrite(motorStat, 2, 1);
      bitWrite(motorStat, 3, 1);
      switch(dirakt){
        case 0:
          zustand=0;
        break;
        case 1:
        if(currentMillis-previousMillis[6]>=thresh){
          previousMillis[6]=currentMillis; 
          zustand=1;
        }
        break;
        case -1:
        if(currentMillis-previousMillis[6]>=thresh){
          previousMillis[6]=currentMillis; 
          zustand=3;
          duty=0;
          analogWrite(mosfet,duty);
        }
        break;
      }
    break;
    case 3:
      bitWrite(motorStat, 0, 1);
      bitWrite(motorStat, 1, 1);
      switch(dirakt){
        case 0:
          zustand=4;
        break;
        case 1:
        if(currentMillis-previousMillis[6]>=thresh){
          previousMillis[6]=currentMillis; 
          zustand=5;
          duty=0;
        }
        break;
        case -1:
        if(currentMillis-previousMillis[6]>=thresh){
          previousMillis[6]=currentMillis; 
          zustand=5;
          duty=0;
        }
        break;
      }
    break;
    case 4:
      bitWrite(motorStat, 0, 0);
      bitWrite(motorStat, 1, 0);
      duty=0;
      analogWrite(mosfet,duty);
      if(currentMillis-previousMillis[6]>=thresh){
        previousMillis[6]=currentMillis; 
        switch(dirakt){
          case 0:
            zustand=0;
          break;
          case 1:
            zustand=0;
          break;
          case -1:
            zustand=0;
          break;
        }
      }
    break;
    case 5:
      if(duty<255){
        if(duty+ramp<=255){
          duty=duty+ramp;
        }
        else duty=255;
        previousMillis[6]=currentMillis;
      }
      analogWrite(mosfet,duty);
      switch(dirakt){
        case 0:
          zustand=4;
        break;
        case 1:
        if(currentMillis-previousMillis[6]>=thresh){
          previousMillis[6]=currentMillis; 
          zustand=6;
          duty=255;
       analogWrite(mosfet,duty); 
        }
        break;
        case -1:
        if(currentMillis-previousMillis[6]>=thresh){
          previousMillis[6]=currentMillis; 
          zustand=6;
          duty=255;
       analogWrite(mosfet,duty); 
        }
        break;
      }
    break;
    case 6:
        /*if(duty>0){
          duty=duty-ramp;
          
          previousMillis[6]=currentMillis;
        }
        analogWrite(mosfet,duty);*/
      switch(dirakt){
        case 0:
          zustand=7;
        break;
        case 1:
          zustand=6;
        break;
        case -1:
          zustand=6;
        break;
      }
    break;
    case 7:
      
      if(currentMillis-previousMillis[6]>=thresh){
        previousMillis[6]=currentMillis; 
        switch(dirakt){
          case 0:
            zustand=4;
          break;
          case 1:
            zustand=4;
          break;
          case -1:
            zustand=4;
          break;
        }
      }
    break;
  }
  /*if (dirakt == 0) {
    digitalWrite(motor1, LOW);
    digitalWrite(motor2, LOW);
    digitalWrite(motorEnable,LOW);
  }
  if (dirakt == 1) {
    digitalWrite(motor1, HIGH);
    digitalWrite(motor2, LOW);
    digitalWrite(motorEnable,HIGH);

  }
  if (dirakt == -1) {
    digitalWrite(motor1, LOW);
    digitalWrite(motor2, HIGH);
    digitalWrite(motorEnable,HIGH);
  }*/
  Wire.beginTransmission(adresse2);
  Wire.write(GPIOA);
  Wire.write(motorStat);
  Wire.endTransmission();
  Serial.print(zustand);
  Serial.print("  ");
  Serial.println(duty);
}

void Belastungstest() {
  if (belastungstest == 1) { //Stufe 1 des Belastungstests, übertragung am PC starten
    Serial.println("startTest");
    Debug(String(belastungstest));
    belastungstest=2;
  }
  if (belastungstest == 2){//Stufe 2 des Belastungstests, variablen vorbereiten
    Debug(String(belastungstest));
    maxiBelastung=0;
    maxiBelastungLast=0;
    grund=0;
    belastungstest=3;
  }
  if (belastungstest==3){//Stufe 3 des Belastungstests, zug aufbauen
    Debug(String(belastungstest));
    dirakt=1;
    belastungstest=4;
  }
  if (belastungstest==4){//Stufe 4 des Belastungstests, auf Bruch oder Sensorüberlastung warten
    Debug(String(belastungstest));
    maxiBelastungLast=maxiBelastung;
    if(tmpWeight>maxiBelastung){
      maxiBelastung=tmpWeight;
    }
    if(maxiBelastung>maxBelastung[sensorType]){ //sensorüberlastung prüfen
      if(maxBelastung[sensorType]!=0){ //bei usr01 bis usr03 keine maxBelastung hinterlegt -> Überlastung kann nicht erkannt werden
        belastungstest=5;
      }
    }
    if(maxiBelastung>=minGew){ //abreißen prüfen
      if((maxiBelastung-tmpWeight)>=((maxiBelastung*0.2)+minGew)){
        belastungstest=6;
      }
    }
  }
  if (belastungstest==5){ //Stufe 5 des Belastungstest, abbruch des tests duch sensorüberlastung
    Debug(String(belastungstest));
    dirakt=0;
    grund=1;
    Serial.println("errorTest: Abbruch, weil Sensorüberlastung");
    doonce[18]=0;
    belastungstest=9;
    
  }
  if (belastungstest==6){ //Stufe 6 des Belastungstest, beenden des test bei Klebstoffbruch
    Debug(String(belastungstest));
    dirakt=0;
    grund=2;
   Serial.println("stopTest");
    belastungstest=8;
    
  }
  if (belastungstest==7){ //Stufe 7 des Belastungstest, abbruch des test durch den user
    Debug(String(belastungstest));
    dirakt=0;
    grund=3;
    Serial.println("errorTest: Abbruch durch Usereingabe");
    belastungstest=8;
  }
  if (belastungstest==8){ //Stufe 8 des Belastungstest, ende der Prüfung, zeigen der testergebnisse, warten auf usereingaben
    Debug(String(belastungstest));
    //belastungstest=0;
  }
  if  (belastungstest==9){
    if(doonce[18]==0){
      dirakt=0;
      doonce[18]=1;
    }
    else{
    if(tmpWeight>(0.5*maxBelastung[sensorType])){
      dirakt=-1;
    }
    else{
      dirakt=0;
      belastungstest=8;
    }}
  }
}

void BelastungstestAbbruch(){
  belastungstest=7;
}

void SerialReceive() { //Empfängt, verarbeitet und verteilt eingehende Befehle von einem Seriellen anschluss
  if (false) {
    if (Serial.available()>0) {
      char temp = Serial.read();
      Serial.println(temp);
      switch (temp){
        case '0':

        break;
        case '1':
          for(int i =0;i<=100;i++);
        break;
        case '2':
          if(menuNr==4)
          belastungstest=1;
        break;
        case '3':
          Serial.print(tmpRaw,0);
          Serial.print(" ");
          Serial.print(tmpOffset,0);
          Serial.print(" ");
          Serial.print(calibration_factor[sensorType],0);
          Serial.print(" ");
          Serial.print(tmpWeight,3);
          Serial.print(" ");
          Serial.print(menuNr);
          Serial.print(" ");
          Serial.print(Sensor[sensorType]);
        break;
      }
      Serial.print(" ");
      Serial.println("Stop");  
    }
  }
}

void SerialSend() { //Sendet Seriell Informationen des Prüfstandes an den PC
  if (SerialExchange) {
    /*Serial.print(tmpRaw,0);
    Serial.print(" ");
    Serial.print(tmpOffset,0);
    Serial.print(" ");
    Serial.print(calibration_factor[sensorType],0);
    Serial.print(" ");*/
    Serial.print((tmpWeight*1000),0);
    //Serial.println("; ");
    Serial.print("  ");
    Serial.print((maxiBelastung*1000),0);
    Serial.print(" ");
    Serial.print((((maxiBelastung*0.8)+minGew)*1000),0);
    Serial.println(";");
  }
}

byte shifting(byte input, int isol) { //isol: zu isolierender Bit
  byte tmp = input << (8 - isol);   /*Zu isolierender Bit wird zuerst an den linken rand des Bytes geschoben, alle werte links davon gehen verlohren*/
  tmp = tmp >> 7;                   /*Zu isolierender Bit wird an den rechten rand des Bytes geschoben, alles rechts des bits geht verlohren*/
  return tmp;                       /*alleiniger Bit, entweder 1 oder 0, wird zurück gegeben*/
}

void save(){
  arrowState=1;
}

void saveAni(){ //animiert eine speicherbestätigung
  switch(arrowState){
    case 0:
    break;
    case 1:
      lcd.setCursor(19,3);
      lcd.write(0);
      arrowState=2;
      previousMillis[4]=currentMillis;
    break;
    case 2:
      if(currentMillis-previousMillis[4]>=arrowDelay){
        lcd.setCursor(19,3);
        lcd.write(1);
        arrowState=3;
        previousMillis[4]=currentMillis;
      }
    break;
    case 3:
      if(currentMillis-previousMillis[4]>=arrowDelay){
        lcd.setCursor(19,3);
        lcd.write(2);
        arrowState=4;
        previousMillis[4]=currentMillis;
      }      
    break;
    case 4:
      if(currentMillis-previousMillis[4]>=arrowDelay){
        lcd.setCursor(19,3);
        lcd.print(" ");
        arrowState=0;
        previousMillis[4]=currentMillis;
      }
    break;
  }
}

void writeEEPROM(int deviceaddress, unsigned int eeaddress, byte data ){
  if(readEEPROM(deviceaddress,eeaddress)!=data){
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(data);
    Wire.endTransmission();
    Debug("written");
    Debug((String)data);
    delay(5);
  }
}
 union u_tag{
    byte bin[4];
    float num;
  }u;
  
void writeFloatEEPROM(int deviceaddress, unsigned int eeaddress, float data ){
    u.num=data;
    Wire.beginTransmission(deviceaddress);
    Wire.write((int)(eeaddress >> 8));   // MSB
    Wire.write((int)(eeaddress & 0xFF)); // LSB
    Wire.write(u.bin[0]);
    Wire.write(u.bin[1]);
    Wire.write(u.bin[2]);
    Wire.write(u.bin[3]);    
    Wire.endTransmission();
    Debug("EEPROM float written");
}

float readFloatEEPROM(int deviceaddress, unsigned int eeaddress){
 
  u.bin[0] = readEEPROM(deviceaddress,eeaddress);
  u.bin[1] = readEEPROM(deviceaddress,eeaddress+1);
  u.bin[2] = readEEPROM(deviceaddress,eeaddress+2);
  u.bin[3] = readEEPROM(deviceaddress,eeaddress+3);

  float out = u.num;
  return out;
  /*rfloat=readEEPROM(deviceaddress,eeddress);
  rfloat=(rfloat << 8);
  rfloat=readEEPROM(deviceaddress,eeaddress+1);
  rfloat=(rfloat << 8);
  rfloat=readEEPROM(deviceaddress,eeaddress+2);
  rfloat=(rfloat << 8);
  rfloat=readEEPROM(deviceaddress,eeaddress+3);
  return rfolat; */ 
}

byte readEEPROM(int deviceaddress, unsigned int eeaddress ){
  byte rdata = 0xFF;
 
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8));   // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
 
  Wire.requestFrom(deviceaddress,1);
 
  if (Wire.available()) rdata = Wire.read();
 
  return rdata;
}

void Debug() {
  if (debug) {
    if (Serial.available()) {
      char temp = Serial.read();
      if (temp == 'd')
        menuPlus();
      else if (temp == 'a')
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

void Debug(String in) {
  if (debug) {
    Serial.println(in);
  }
}

void LCDSimul(String in) {
  if (LCDSimulation == true) {
    String comp = "n";
    if (in == comp) {
      Serial.println(" ");
    }
    else {
      Serial.print(in);
    }
  }
}
