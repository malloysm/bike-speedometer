#include <LiquidCrystal.h>

#include <EEPROM.h>

// EEPROM variables
float  EEMEM circStore;
float  EEMEM miStore;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
float mi = 0;
float n = 0; //Used to update mi display only every ~.5 mi
float k = 0; //For updating mi storage every ~.01 mi

const int reed = 7; // Pin connected to reed switch

unsigned long spdTimer = 0; // Timer used to update speed display once every 1 second
unsigned long magnetTimer = 0; // Timer for debouncing magnet
unsigned long revTimer = 0; // Times revolutions

float rev = 0;
float spd = 0;

const int buttonA = 9;
const int buttonB = 8;

//used for circumference collection
int lcdValue = 0; // screen position of circumference place value
int multiplier = 0; // for adding together individual circumference place values
int number = 0;
float circ = 90.00; // circumference value for calculations

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set up reed switch
  pinMode(reed, INPUT_PULLUP);

  // initialize buttons
  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);

  // retrieve stored circumference & mi
  eeprom_read_block(&circ, &circStore, sizeof(circ));
  eeprom_read_block(&mi, &miStore, sizeof(mi));
}

void loop() {
  // Receive tire circumference input
  if (multiplier == 0) {
    if (digitalRead(buttonB) == HIGH) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Alter Tire Size?");
      lcd.setCursor(0, 1);
      lcd.print("Current: ");
      lcd.print(circ);
      lcd.print("in");
      multiplier = -1; 
      delay(500);
    } 
  }
  else if (multiplier == -1) {
    if (digitalRead(buttonB) == HIGH) {
      lcd.clear();
      multiplier = 0;
      delay(500);

      lcd.setCursor(0, 1);
      lcd.print("Distance      mi");
      lcd.setCursor(9, 1);
      lcd.print(mi);
    }
    else if (digitalRead(buttonA) == HIGH) {
      multiplier = -2;
      delay(500);
    }
  }
  else if (multiplier == -2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Circumference of");
    lcd.setCursor(0, 1);
    lcd.print("Tire:    .   in");
    lcdValue = 7;
    lcd.setCursor(lcdValue, 1);
    lcd.print("0");
    circ = 0;
    multiplier = 1;
    delay(500);
  }
  else if (digitalRead(buttonB) == HIGH) {
    if (number == 9) {
      number = 0;
    }
    else {
      number = number + 1;
    }
    lcd.setCursor(lcdValue, 1);
    lcd.print(number);
    delay(500);
  }
  else if (digitalRead(buttonA) == HIGH) {
    if (lcdValue == 8) {
      lcdValue = 10;
    }
    else {
      lcdValue = lcdValue + 1;
    }
    circ = circ + ((1.0/((float)multiplier)) * 10.0 * (float)number);
    multiplier = multiplier * 10;
    lcd.setCursor(lcdValue, 1);
    if (lcdValue == 12) {
      multiplier = 0;
      lcd.clear();

      lcd.setCursor(0, 1);
      lcd.print("Distance      mi");
      lcd.setCursor(9, 1);
      lcd.print(mi);
      
      // store circumference to memory
      eeprom_write_block(&circ, &circStore, sizeof(circ)); 
    }
    else lcd.print("0");
    number = 0;
    delay(500);
  }



  if (multiplier == 0) {
    // Determine if magnet passes switch
    int reedMagnet = digitalRead(reed);
    
    if (magnetTimer == 0 && reedMagnet == LOW) {
      mi = mi + (circ * .0000157828);
      magnetTimer = millis();
      rev = (float)(millis() - revTimer);
      spd = (circ * .0000157828) / (rev * .0000002778);
      revTimer = 0;
      revTimer = millis();
    }
    else if (millis()-magnetTimer >= 200UL) {
       magnetTimer = 0;
    }
    
  
    if (spdTimer == 0) {
      spdTimer = millis();
    }
  
  
    
    // Print a message to the LCD.
    if (millis()-spdTimer >= 1000UL) {
      lcd.setCursor(0, 0);
      lcd.print("Speed        mph");
      lcd.setCursor(7, 0);
      lcd.print(spd);
  
      spdTimer = 0;
    }
  
    if (mi >= n) {
      lcd.setCursor(0, 1);
      lcd.print("Distance      mi");
      lcd.setCursor(9, 1);
      lcd.print(mi);
      n = n + 0.5;
    }

    if (mi >= k) {
      // update mi storage to memory
      eeprom_update_block(&mi, &miStore, sizeof(mi));

      k = k + 0.01;
    }
  }
}

