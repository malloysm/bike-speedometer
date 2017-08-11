#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
float mi = 0;
float n = 0; //Used to update mi display only every ~.5 mi

const int reed = 7; // Pin connected to reed switch

unsigned long spdTimer = 0; // Timer used to update speed display once every 1 second
unsigned long magnetTimer = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set up reed switch
  pinMode(reed, INPUT_PULLUP);
}

void loop() {
  // Determine if magnet passes switch
  int reedMagnet = digitalRead(reed);
  
  if (magnetTimer == 0 && reedMagnet == LOW) {
    mi = mi + 0.123;
    magnetTimer = millis();
  }
  else if (millis()-magnetTimer >= 1000UL) {
     magnetTimer = 0;
  }
  
  float randSpd = random(0,30);

  if (spdTimer == 0) {
    spdTimer = millis();
  }
  
  // Print a message to the LCD.
  if (millis()-spdTimer >= 1000UL) {
    lcd.setCursor(0, 0);
    lcd.print("Speed        mph");
    lcd.setCursor(7, 0);
    lcd.print(randSpd);

    spdTimer = 0;
  }

  if (mi >= n) {
    lcd.setCursor(0, 1);
    lcd.print("Distance      mi");
    lcd.setCursor(9, 1);
    lcd.print(mi);
    n = n + 0.5;
  }
}

