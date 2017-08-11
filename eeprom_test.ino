// http://www.avrfreaks.net/forum/how-readwrite-float-eeprom

#include <EEPROM.h>
// EEPROM variables
float  EEMEM eefloat;
float EEMEM circStore;

float f2 = 0;
int a = 0;

void setup() {
  // Writing & Reading EEPROM:
  Serial.begin(9600);
  float f1 = 1.234;

  Serial.println(f2);
  eeprom_write_block(&f1, &eefloat, sizeof(f1)); 
  delay(5000);
  eeprom_read_block(&f2, &eefloat, sizeof(f2)); 
  Serial.println(f2);

  f1 = 195.00076;
  eeprom_write_block(&f1, &circStore, sizeof(f1)); 
  delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  if (a == 0) {
    eeprom_read_block(&f2, &circStore, sizeof(f2)); 
    Serial.println(f2);
    a = 1;
  }
}
