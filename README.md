# Klebstoffpruefstand

  Nötige libaries:
    #include <LiquidCrystal_I2C.h>
    #include <HX711.h>
    #include <Wire.h>
    #include <EEPROM.h>

  Software zur Verwendung mit einem Klebstoff-Prüfstand
  
  
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
          DT      -   Definierter Arduino Pin DT (5)
          SCK     -   Definierter Arduino Pin SCK(6)
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
