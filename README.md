# Klebstoffpruefstand

  Nötige libaries:
    #include <LiquidCrystal_I2C.h>
    #include <HX711.h>
    #include <Wire.h>

  Software zur Verwendung mit einem Klebstoff-Prüfstand
  
  
  Anschlussanweisungen:
     HX711 Modul:
       Wägezelle zu HX711:
         Rot     -   E+
         Schwarz -   E-
         Grün    -   A-
         Weiß    -   A+
       Modul zu Arduino:
         GND     -   GND
         VCC     -   5V
         DT      -   Definierter Arduino Pin DT
         SCK     -   Definierter Arduino Pin SCK
     Bedienelement:
       MCP23017 zu Arduino:
         Pin 9 |VDD    -   5V
         Pin 18|RESET  -   5V
         Pin 10|VSS    -   GND
         Pin 12|SCL    -   A5|SCL
         Pin 13|SDA    -   A4|SDA
         I²C-Addresse über Pin 15 - Pin 17 am MCP23017 durch verbinden mit GND oder 5V, alle 3 zu GND: Adresse 0x20/100000 
       Taster zu MCP23017:
         Über Pulldown-Wiederstände geschaltet: 5V an ein Buttonterminal, Verbindung zu MCP23017 an anderes Terminal + Verbindung zu  GND über hochohmigen Wiederstand.
         OK    -   GPB0|Pin 1
         UP    -   GBP1|Pin 2
         DOWN  -   GBP2|Pin 3
         LEFT  -   GBP3|Pin 4
         RIGHT -   GBP4|Pin 5
         TARE  -   GBP5|Pin 6
