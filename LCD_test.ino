#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
int mi = 0;

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
}

void loop() {
  int randSpd = random(0,30);
  mi = mi + 1;
  // Print a message to the LCD.

  lcd.print("Speed        mph");
  lcd.setCursor(7, 0);
  lcd.print(randSpd);
  lcd.setCursor(0, 1);
  lcd.print("Distance      mi");
  lcd.setCursor(9, 1);
  lcd.print(mi);
  lcd.setCursor(0, 0);
  delay(1000);
}

