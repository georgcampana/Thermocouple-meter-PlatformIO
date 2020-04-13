#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>


const int encoder_skpin = DD2;
const int encoder_sk1pin = DD3;
const int encoder_key = DD4;

const int display_i2csda = SDA;
const int display_i2cscl = SCL;

const int thermosensor_spisck = SCK;
const int thermosensor_spimosi = MOSI;
const int thermosensor_spics = SS;

// make a cute degree symbol
uint8_t degree[8]  = {140,146,146,140,128,128,128,128};


Encoder inputRotary(encoder_skpin, encoder_sk1pin);
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
MAX6675 thermocouple;


int modepage = 0; //mainloop
uint32_t lastchange_time = 0;
uint32_t looptime;
double currenttemp;
double maxtemp;
double mintemp;
double offsettemp=0;

void setup() {

  pinMode(encoder_key, INPUT);
  
  inputRotary.write(0);
  
  lcd.init();                       // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Georg's Thermo!");

  lcd.createChar(0, degree);

  thermocouple.begin(thermosensor_spisck,thermosensor_spics,thermosensor_spimosi);

  lastchange_time = millis();

}

int executedsec;
void mainloop() {

  // we do the stuff only once per sec 
  int elapsed_secs = (looptime - lastchange_time) / 1024;
  if(elapsed_secs == executedsec) return;
  executedsec = elapsed_secs;

  // temp reading loop
  if(elapsed_secs & 1) { // we read at odd 
    currenttemp = thermocouple.readCelsius();
    if(currenttemp > maxtemp) maxtemp = currenttemp;
    if(currenttemp < mintemp) mintemp = currenttemp;
  }
  else { // and display at even
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("< Current Temp >");
    // go to line #1
    lcd.setCursor(1,1);
    lcd.print('C');
    lcd.write((byte)0);
    lcd.print(currenttemp);
  }
}

void viewmaxloop() {
  // we do the stuff only once per sec 
  int elapsed_secs = (looptime - lastchange_time) / 2048;
  if(elapsed_secs == executedsec) return;
  executedsec = elapsed_secs;
 
  // show max 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Max T (reset)>");
  // go to line #1
  lcd.setCursor(1,1);
  lcd.print('C');
  lcd.write((byte)0);
  lcd.print(maxtemp);
}

void viewminloop() {

  // we do the stuff only once per 2 secs 
  int elapsed_secs = (looptime - lastchange_time) / 1024;
  if(elapsed_secs == executedsec) return;
  executedsec = elapsed_secs;
 
  // show min 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Min T (reset)>");
  // go to line #1
  lcd.setCursor(1,1);
  lcd.print('C');
  lcd.write((byte)0);
  lcd.print(mintemp);
}

void setOffsetloop() {
  // we do the stuff only once per 2 secs 
  int elapsed_secs = (looptime - lastchange_time) / 1024;
  if(elapsed_secs == executedsec) return;
  executedsec = elapsed_secs;
 
  // show min 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Offset T (set)  >");
  // go to line #1
  lcd.setCursor(1,1);
  lcd.print('C');
  lcd.write((byte)0);
  lcd.print(offsettemp);
}

void aboutloop() {
  // we do the stuff only once per 2 secs 
  int elapsed_secs = (looptime - lastchange_time) / 1024;
  if(elapsed_secs == executedsec) return;
  executedsec = elapsed_secs;
 
  // show min 
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<    About!    >");
  // go to line #1
  lcd.setCursor(0,1);
  lcd.print("Vers: 1.0");
}


uint8_t light = 1;
void loop() {

  looptime = millis();

  int newvalue = inputRotary.read() >> 2; // divide by 4
  if(newvalue == modepage) {
    // anything to do globally due to idle status (e.g. light off)
    if(looptime - lastchange_time > 5000) {
      lcd.setBacklight(0);
      light = 0;
      lastchange_time = looptime;
    }
  }
  else {
    modepage = newvalue;
    lastchange_time = looptime;
    executedsec = 0; // make sure the stuff gets executed when changing loop
    if(light == 0) {
      lcd.setBacklight(1);
      light = 1; 
    }  
  }

  // loops are called only if: 1 sec is elapsed, 
  
  switch(modepage) {

    case 0:
      mainloop();
      break;
    case 1:
      viewmaxloop();
      break;
    case 2:
      viewminloop();
      break;
    case 3:
      setOffsetloop();
      break;
    case 4:
      aboutloop();
      break;
    default:
      if(modepage > 4) modepage = 0;
      if(modepage < 0) modepage = 4;
      inputRotary.write(modepage<<2); // seems that the value is allways a multiple of 4 so we multiply the pageno by 4
  }

}