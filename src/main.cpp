#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <max6675.h>
#include <EEPROM.h>

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
uint32_t lastinputchange_time = 0;
uint32_t looptime; // kept global to allow other subloops to access the current loop time
double currenttemp;
double maxtemp;
double mintemp;
double offsettemp=0;

void setup() {

  int offset_int;
  EEPROM.get(0,offset_int); // let's read from address 0
  if(offset_int > 50 || offset_int < -50) {
    // let's init properly
    EEPROM.write(0,((int)(offsettemp*4))) ;
  }
  else {
    offsettemp = offset_int / 4;
  }

  pinMode(encoder_key, INPUT);
  
  inputRotary.write(0);
  
  lcd.init();                       // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Georg's Thermo!");

  lcd.createChar(0, degree);

  thermocouple.begin(thermosensor_spisck,thermosensor_spics,thermosensor_spimosi);

  lastinputchange_time = millis();

  mintemp = 2000;
  maxtemp = -2000; // dummy values out of the sensor range

}

uint32_t elapsed_secs;
uint32_t executed_sec;

bool mainloop(bool button) {

  // temp reading loop
 
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Thermocouple >");
  // go to line #1
  lcd.setCursor(0,1);
  lcd.print(" C");
  lcd.write((byte)0);
  lcd.print(currenttemp);
  lcd.print("        ");


  return false;
}

bool viewmaxloop(bool button) {

  if(button) maxtemp = currenttemp; 
  // show max 
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Max T (reset)>");
  // go to line #1
  lcd.setCursor(0,1);
  lcd.print(" C");
  lcd.write((byte)0);
  lcd.print(maxtemp);
  lcd.print("        ");
  return false;
}

bool viewminloop(bool button) {

  if(button) mintemp = currenttemp;

  // show min 
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("< Min T (reset)>");
  // go to line #1
  lcd.setCursor(0,1);
  lcd.print(" C");
  lcd.write((byte)0);
  lcd.print(mintemp);
  lcd.print("        ");
  return false;
}

bool editoffset = false;
bool setOffsetloop(bool button) {

  if(button) {
    editoffset = !editoffset;
    if(editoffset) {
      inputRotary.write((int32_t)(offsettemp * 16)); // encoder is 4 per tick and max6675 has 4 values per 1 C
    }
    else {
      offsettemp = (double)(inputRotary.read()) / 16;
      EEPROM.write(0,((int)(offsettemp*4))) ;
      inputRotary.write(modepage * 4);
      lcd.clear();
    }
  }    

  if(editoffset) {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Edit offset(set)");
    // go to line #1
    lcd.setCursor(0,1);
    lcd.print("<-->  C");
    lcd.write((byte)0);
    lcd.print((double)(inputRotary.read()) / 16);
    lcd.print("        ");
  }
  else {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("<Offset T (set)>");
    // go to line #1
    lcd.setCursor(0,1);
    lcd.print(" C");
    lcd.write((byte)0);
    lcd.print(offsettemp);
    lcd.print("         ");
  }
  return editoffset;
}

bool aboutloop(bool button) {
  //lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<    About!    >");
  // go to line #1
  lcd.setCursor(0,1);
  lcd.print("Vers:1.0 - Georg");

  return false;
}

bool editreset = false;
int currentrotaryval;
bool resetloop(bool button) {
  if(button) {
    editreset = !editreset;

    if(editreset) {
      currentrotaryval = inputRotary.read();
    }
    else {
      offsettemp = 0;
      mintemp = 2000;
      maxtemp = -2000; // dummy values out of the sensor range

      EEPROM.write(0,offsettemp);
    }
  }

  if(editreset) {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("<    Reset!    >");
    // go to line #1
    lcd.setCursor(0,1);
    lcd.print("CONFIRM or move!");
    if(inputRotary.read() != currentrotaryval) {
      // moved to escape confirmation
      inputRotary.write(currentrotaryval);
      editreset = false;
    }
  }
  else {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("<    Reset!    >");
    // go to line #1
    lcd.setCursor(0,1);
    lcd.print("-Press to reset-");
  }

  return editreset;
}

uint8_t light = 1;
bool buttonstatus = false;
uint32_t lastbutton_ts;

bool modepage_locked = false;
int lastrotary_val = 0;

void loop() {

  bool encoder_changed = false;
  bool second_changed = false;
  bool button_changed = false;

  looptime = millis();
  elapsed_secs = looptime / 1024; // not really precise but quite fast

  if(executed_sec != elapsed_secs) {
    second_changed = true;
  }

  if((looptime - lastbutton_ts) > 200) { // debounce

    bool buttonvalue = (digitalRead(encoder_key) == LOW) ? true:false; // NOTE: inverted: my button is "normally high"
    if(buttonvalue != buttonstatus) {
      lastbutton_ts = looptime;
      buttonstatus = buttonvalue;
      button_changed = true;
      lastinputchange_time = looptime;
    }
  }
  
  int newvalue = inputRotary.read() >> 2; // divide by 4
  if(newvalue != lastrotary_val) {
    lastrotary_val = newvalue;
    encoder_changed = true;
    lastinputchange_time = looptime;
    if(!modepage_locked){
      if(newvalue != modepage) {
        modepage = newvalue;
      }
    }
  }
  else {
    // anything to do globally due to idle status (e.g. light off)
    if(looptime - lastinputchange_time > 5000) {
      lcd.setBacklight(0);
      light = 0;
      lastinputchange_time = looptime;
    }
  }

  // loops are called only if: 1 sec is elapsed, the modepage changed or the button has been pushed / released
  if(!second_changed && !button_changed && !encoder_changed  ) {
    return; // nothing to do
  }
  
  if((button_changed || encoder_changed) && light == 0) {
    lcd.setBacklight(1);
    light = 1; 
  }

  {
    currenttemp = thermocouple.readCelsius();
    currenttemp += offsettemp;
    if(currenttemp > maxtemp) maxtemp = currenttemp;
    if(currenttemp < mintemp) mintemp = currenttemp;
  }
  executed_sec = elapsed_secs;

  switch(modepage) {

    case 0:
      modepage_locked = mainloop(buttonstatus);
      break;
    case 1:
      modepage_locked = viewmaxloop(buttonstatus);
      break;
    case 2:
      modepage_locked = viewminloop(buttonstatus);
      break;
    case 3:
      modepage_locked = setOffsetloop(buttonstatus);
      break;
    case 4:
      modepage_locked = aboutloop(buttonstatus);
      break;
    case 5:
      modepage_locked = resetloop(buttonstatus);
      break;
    default:
      if(modepage > 5) modepage = 0;
      if(modepage < 0) modepage = 5;
      inputRotary.write(modepage<<2); // seems that the value is allways a multiple of 4 so we multiply the pageno by 4
  }

}