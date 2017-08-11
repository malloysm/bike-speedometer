#include <LiquidCrystal.h>

#include <EEPROM.h>

// EEPROM variables
float  EEMEM circStore;
float  EEMEM miStore;
float  EEMEM timeStore;
float  EEMEM tripStore;

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
float mi = 0;
float n = 0; //Used to update mi display only every ~.5 mi
float k = 0; //For updating mi storage every ~.01 mi

const int reed = 7; // Pin connected to reed switch

unsigned long spdTimer = 0; // Timer used to update speed display once every 1 second
unsigned long magnetTimer = 0; // Timer for debouncing magnet
unsigned long revTimer = 0; // Times revolutions
unsigned long tripTimer = 0; // Times the trip

float rev = 0; // time taken for 1 revolution
float spd = 0;

const int buttonA = 9;
const int buttonB = 8;

//used for circumference collection
int lcdValue = 0; // screen position of circumference place value
int multiplier = 0; // for adding together individual circumference place values
int number = 0;
float circ = 90.00; // circumference value for calculations

int displayMode = 0; // for displaying speed + distance or trip time + trip distance

int tripTime = 0; // time of trip in minutes, does not increment when speed 0
float tripDist = 0;

int hour = 0;
int minute = 0;
int j = 0; //for updating clock

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // set up reed switch
  pinMode(reed, INPUT_PULLUP);

  // initialize buttons
  pinMode(buttonA, INPUT);
  pinMode(buttonB, INPUT);

  
  
  // retrieve stored circumference, mi, and trip data
  eeprom_read_block(&circ, &circStore, sizeof(circ));
  eeprom_read_block(&mi, &miStore, sizeof(mi));

  eeprom_read_block(&tripTime, &timeStore, sizeof(tripTime));
  eeprom_read_block(&tripDist, &tripStore, sizeof(tripDist));
}

void loop() {
  // default display and button modes
  if (multiplier == 0) {
    // toggles between trip display and speed display when button A pressed
    if (digitalRead(buttonA) == HIGH) {
      lcd.clear();
      delay(500);
      n = 0;
      if (displayMode == 0) {
        displayMode = 1;
        lcd.setCursor(0, 0);
        
        j = 1;
      }
      else {
        displayMode = 0;     
      }
    }
    // if button B pressed, displays current circumference and asks if want to change it
    else if (digitalRead(buttonB) == HIGH) {
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
  // actions available after pressing button B from default display. answers if tire size should be changed
  else if (multiplier == -1) {
    // if button B pressed then leave circumference same and ask if trip data should be reset
    if (digitalRead(buttonB) == HIGH) {
      lcd.clear();
      multiplier = -2;
      delay(500);

      lcd.setCursor(0, 0);
      lcd.print("Reset Trip Data?");
    }
    // if button A pressed then proceed to changing circumference
    else if (digitalRead(buttonA) == HIGH) {
      multiplier = -3;
      delay(500);
    }
  }
  // actions available after pressing button B from change tire size display. answers if trip data should be reset
  else if (multiplier == -2) {
    // if button B pressed don't reset trip data, return to default display
    if (digitalRead(buttonB) == HIGH) {
      lcd.clear();
      multiplier = 0;
      delay(500);

      n = 0;
      j = 1;
    }
    // if button A pressed trip data is reset, return to default display
    else if (digitalRead(buttonA) == HIGH) {
      multiplier = 0;
      delay(500);

      tripTime = 0;
      tripDist = 0;
      n = 0;
      j = 1;

      // saves 0's into data for trip time and distance
      eeprom_write_block(&tripTime, &timeStore, sizeof(tripTime)); 
      eeprom_write_block(&tripDist, &tripStore, sizeof(tripDist)); 
    }
  }
  // actions for changing tire circumference, moves from MSB to LSB 00.00
  else if (multiplier == -3) {
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
  // pressing B counts through numbers 0-9
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
  // pressing A advances to next digit of circumference
  else if (digitalRead(buttonA) == HIGH) {
    if (lcdValue == 8) {
      lcdValue = 10;
    }
    else {
      lcdValue = lcdValue + 1;
    }
    // uses entered numbers to make circumference for calculations
    circ = circ + ((1.0/((float)multiplier)) * 10.0 * (float)number);
    multiplier = multiplier * 10;
    lcd.setCursor(lcdValue, 1);
    // after hundreths place is set for circumference, save circumference and return to default display
    if (lcdValue == 12) {
      multiplier = 0;
      lcd.clear();

      n = 0;
      j = 1;
      
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

    // performs code if magnet sensed and 200 milliseconds have passed since last time magnet was sensed
    if (magnetTimer == 0 && reedMagnet == LOW) {
      // add to total mileage
      mi = mi + (circ * .0000157828);
      // add to trip mileage distance
      tripDist = tripDist + (circ * .0000157828);
      magnetTimer = millis();
      // amount of time 1 revolution took
      rev = (float)(millis() - revTimer);
      // calculate speed from revolution time and circumference in mph
      spd = (circ * .0000157828) / (rev * .0000002778);
      revTimer = 0;
      revTimer = millis();
    }
    else if (millis() - magnetTimer >= 200UL) {
       magnetTimer = 0;
    }

    // if a revolution takes more than 10 seconds, set speed to 0.
    if (millis() - revTimer >= 10000UL) {
      spd = 0;
      revTimer = 0;
    }

    // display speed and total miles
    if (displayMode == 0) {
      if (spdTimer == 0) {
        spdTimer = millis();
      }
    
      
      // updates speed display to LCD every second
      if (millis()-spdTimer >= 1000UL) {
        lcd.setCursor(0, 0);
        lcd.print("Speed        mph");
        lcd.setCursor(7, 0);
        lcd.print(spd);
    
        spdTimer = 0;
      }

      // updates distance display to LCD every 0.5 miles
      if (mi >= n) {
        lcd.setCursor(0, 1);
        lcd.print("Dist");
        lcd.setCursor(6, 1);
        lcd.print(mi);
        lcd.setCursor(14, 1);
        lcd.print("mi");
        n = n + 0.5;
      }
    }
    // display trip time and trip mileage
    else {
      // prints trip time to LCD display
      if (j == 1) {
        // get hours and minutes of trip
        minute = tripTime % 60;
        hour = tripTime / 60;
        
        lcd.setCursor(0, 0);
        lcd.print("Ride Time  00:00");
        if (hour < 10) {
          lcd.setCursor(12, 0);
        }
        else {
          lcd.setCursor(11, 0);
        }
        lcd.print(hour);
        if (minute < 10) {
          lcd.setCursor(15, 0);
        }
        else {
          lcd.setCursor(14, 0);
        }
        lcd.print(minute);

        j = 0;
      }

      // updates trip distance to LCD display every 0.5 miles
      if (tripDist >= n) {
        lcd.setCursor(0, 1);
        lcd.print("Trip");
        lcd.setCursor(6, 1);
        lcd.print(tripDist);
        lcd.setCursor(14, 1);
        lcd.print("mi");
        n = n + 0.5;
      }
    }

    // counts in minutes
    if (millis() - tripTimer >= 60000UL) {
      tripTimer = 0;
      tripTimer = millis();

      // only increment trip minutes when bicycle moving, save to memory each time updated
      if (spd != 0) {
        tripTime = tripTime + 1;
        eeprom_update_block(&tripTime, &timeStore, sizeof(tripTime));
  
        j = 1;
      }
    }

    // saves trip and total distance to memory every .01 miles
    if (mi >= k) {
      // update mi storage to memory
      eeprom_update_block(&mi, &miStore, sizeof(mi));
      eeprom_update_block(&tripDist, &tripStore, sizeof(tripDist)); 
      k = k + 0.01;
    }
  }
}

