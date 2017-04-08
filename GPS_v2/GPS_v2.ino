#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <EEPROMAnything.h>

int test = 0;

// initialize the LCD with the numbers of the interface pins
LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

// initialize the gps
SoftwareSerial gpsSerial(14, 1); // RX, TX (TX not used)
const int sentenceSize = 80;
const int lcdLightPin = 2;

// rf Pins
int txD0 = 9;
int txD1 = 10;
int txD2 = 11;
int txD3 = 12;
bool sendRF = false;
int tx0State = LOW;
long rfOnTime = 0;
long rfOffTime = 0;
int rfState = 0;
int transFreq = 5000;// 2 seconds
int countWonkyON = 99;
int countWonkyOFF = 99;
bool setWonky = false;

//Pushbuttons
int leftButtonState;
int rightButtonState;
int lastLeftButtonState = LOW;
int lastRightButtonState = LOW;
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

//GPS
char sentence[sentenceSize];
bool showGPS = true;
bool transmit = false;
int gpsCoordsIndex = -1;
int EEPROMAdress = 0;
const int arraySize = 61;
long currGpsCoordsLong = 0;
long currGpsCoordsLat = 0;
struct config_t
{
  long gpsCoordsLong [arraySize];
  long gpsCoordsLat [arraySize];
} gpsCaptureData;

int count = 0;
int menuToggle = 0;
int menuSelect = 0;
const int menuSize = 6;

void setup() {
  //RF Transmitter - TX
  pinMode(txD0, OUTPUT);
  pinMode(txD1, OUTPUT);
  pinMode(txD2, OUTPUT);
  pinMode(txD3, OUTPUT);
  pinMode(A3, OUTPUT);//TX Power Pin

  //Pushbuttons
  pinMode(A5, INPUT);
  pinMode(A4, INPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(20, 4);

  pinMode(lcdLightPin, OUTPUT);
  digitalWrite(lcdLightPin, HIGH);

  doMenuToggle();
  doMenuSelect();

  gpsSerial.begin(9600);

  EEPROM_readAnything(EEPROMAdress, gpsCaptureData);//Read AB Data
}

void loop() {
  checkGPS();

  if (sendRF)
    rfTransmit();

  rightButtonRead();
  leftButtonRead();
}

void rfTransmit() {
  int pos = showTrackPos();

  if ((millis() - rfOffTime) > transFreq) {

    if (setWonky == false && countWonkyOFF < 8) {
      countWonkyOFF++;
    }

    if (setWonky == true && countWonkyON < 8) {
      countWonkyON++;
    }

    if (test < 60) { //Just test code!!!!!!!!!!!!!!!!!!!!
      test = test + 1;
    } else {
      test = 0;
    }

    rfState = 0;

    rfOffTime = millis();
  }

  if ((millis() - rfOnTime) > 500) {

    switch (rfState) {
      case 0://1st Nibble
        digitalWrite(A3, HIGH);//Power TX ON
        lcd.setCursor(16, 2);
        lcd.print("HIGH");
        tx0State = HIGH;

        //sendDecToBin(pos);//Set digital pins to BIN
        sendDecToBin(pos);

        rfState = 1;
        break;
      case 1://Low
        digitalWrite(A3, LOW);//Power TX OFF
        lcd.setCursor(16, 2);
        lcd.print(" LOW");

        rfState = 2;
        break;
      case 2://Second Nibble
        digitalWrite(A3, HIGH);//Power TX ON
        lcd.setCursor(16, 2);
        lcd.print("HIGH");
        tx0State = HIGH;
        tx0State = LOW;

        //sendDecToBin(pos);//Set digital pins to BIN
        sendDecToBin(pos);

        rfState = 3;
        break;
      case 3://Low
        digitalWrite(A3, LOW);//Power TX OFF
        lcd.setCursor(16, 2);
        lcd.print(" LOW");
        break;
    }
    rfOnTime = millis();
  }
}


int showTrackPos() {
  float dist = 0;

  int pos;
  if (setWonky == true && countWonkyON < 8) {
    pos = 62;
    lcd.setCursor(13, 3);
    lcd.print("w ");
    lcd.print(countWonkyON);
  } else if (setWonky == false && countWonkyOFF < 8) {
    pos = 63;
    lcd.setCursor(13, 3);
    lcd.print("_w ");
    lcd.print(countWonkyON);
  } else {
    pos = test;//NN(currGpsCoordsLat, currGpsCoordsLong, dist);
  }

  lcd.setCursor(0, 2);
  lcd.print("Position = ");
  lcd.print(pos);
  //  lcd.setCursor(0, 3);
  //  lcd.print("Dist = ");
  //  lcd.print(dist);
  return pos;
}

void sendDecToBin(int dec) {

  String bin = String(dec, BIN);

  if (bin.length() < 6) { //Zero padding binary Nr.
    String zeros = "";
    for (int i = 0; i < (6 - bin.length()); i++) {
      zeros = zeros + "0";
    }
    bin = zeros + bin;
  }

  digitalWrite(txD1, LOW);//Set all bits LOW
  digitalWrite(txD2, LOW);
  digitalWrite(txD3, LOW);
  lcd.setCursor(19, 3);
  lcd.print(tx0State);

  if (tx0State == LOW) {// First nibble
    digitalWrite(txD0, LOW);
    if (bin.substring(0, 1).toInt() == 1)//Bit 1
      digitalWrite(txD1, HIGH);

    if (bin.substring(1, 2).toInt() == 1)//Bit 2
      digitalWrite(txD2, HIGH);

    if (bin.substring(2, 3).toInt() == 1)//Bit 3
      digitalWrite(txD3, HIGH);
  }

  if (tx0State == HIGH) {//Second nibble
    digitalWrite(txD0, HIGH);
    if (bin.substring(3, 4).toInt() == 1)//Bit 4
      digitalWrite(txD1, HIGH);

    if (bin.substring(4, 5).toInt() == 1)//Bit 5
      digitalWrite(txD2, HIGH);

    if (bin.substring(5, 6).toInt() == 1)//Bit 6
      digitalWrite(txD3, HIGH);
  }

  lcd.setCursor(0, 3);
  lcd.print("BIN = ");
  lcd.print(bin);
}

int NN(long latitude, long longitude, float &dist) {
  int NN = 0;
  dist = 999999;
  for (int i = 1; i < arraySize; i++) {
    float elementDist = sqrt(sq(gpsCaptureData.gpsCoordsLat [i] - latitude) + sq(gpsCaptureData.gpsCoordsLong [i] - longitude)); // Calculating the Euclidean distance
    if (dist > elementDist) {
      NN = i;
      dist = elementDist;
    }
  }
  return NN;
}

void checkGPS() {

  static int i = 0;
  if (gpsSerial.available())
  {
    char ch = gpsSerial.read();
    if (ch != '\n' && i < sentenceSize)
    {
      sentence[i] = ch;
      i++;
    }
    else
    {
      sentence[i] = '\0';
      i = 0;
      displayGPS();
    }
  }
}

void displayGPS()
{
  char field[20];
  getField(field, 0);
  if (strcmp(field, "$GPRMC") == 0)
  {
    lcd.setCursor(0, 2);
    if (showGPS == true)
      lcd.print("Lat:   ");
    getField(field, 3);  // number
    if (showGPS == true)
      lcd.print(field);
    currGpsCoordsLat = (String(field).substring(0, 4) + String(field).substring(5)).toInt();
    getField(field, 4); // N/S
    if (showGPS == true)
      lcd.print(field);

    lcd.setCursor(0, 3);
    if (showGPS == true)
      lcd.print("Long: ");
    getField(field, 5);  // number
    if (showGPS == true)
      lcd.print(field);
    currGpsCoordsLong = (String(field).substring(1, 5) + String(field).substring(6)).toInt();
    getField(field, 6);  // E/W
    if (showGPS == true)
      lcd.print(field);
  }
}

void getField(char* buffer, int index)
{
  count++;
  int sentencePos = 0;
  int fieldPos = 0;
  int commaCount = 0;
  while (sentencePos < sentenceSize)
  {
    if (sentence[sentencePos] == ',')
    {
      commaCount ++;
      sentencePos ++;
    }
    if (commaCount == index)
    {
      buffer[fieldPos] = sentence[sentencePos];
      fieldPos ++;
    }
    sentencePos ++;
  }
  buffer[fieldPos] = '\0';
}


void leftButtonRead() {

  int reading = digitalRead(A5);

  if (reading != lastLeftButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != leftButtonState) {
      leftButtonState = reading;
      if (leftButtonState == HIGH) {
        leftButtonState = digitalRead(A5);
        if (menuToggle < menuSize) {
          menuToggle++;
        } else {
          menuToggle = 0;
        }
        doMenuToggle();
      }
    }
  }
  lastLeftButtonState = reading;
}

void rightButtonRead() {

  int reading = digitalRead(A4);

  if (reading != lastRightButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != rightButtonState) {
      rightButtonState = reading;
      if (rightButtonState == HIGH) {
        rightButtonState = digitalRead(A4);
        doMenuSelect();
      }
    }
  }
  lastRightButtonState = reading;
}

void doMenuToggle () {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hauptbahnhof GPS");
  switch (menuToggle) {
    case 0:
      lcd.setCursor(0, 1);
      lcd.print("------Standby------");
      break;
    case 1:
      lcd.setCursor(0, 1);
      lcd.print("-----Speed Mode----");
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print("-----Step Mode-----");
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print("----Capture Mode---");
      break;
    case 4:
      lcd.setCursor(0, 1);
      lcd.print("----Saved Data-----");
      break;
    case 5:
      lcd.setCursor(0, 1);
      lcd.print("-Current Track Pos-");
      break;
    case 6:
      lcd.setCursor(0, 1);
      lcd.print("-Send Current Pos--");
      break;
  }
}

void doMenuSelect () {
  sendRF = false;
  menuSelect = menuToggle;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hauptbahnhof GPS");
  lcd.setCursor(0, 1);
  switch (menuSelect) {
    case 0:
      showGPS = true;
      transmit = true;
      digitalWrite(txD0, HIGH);//Wait mode
      digitalWrite(txD1, LOW);
      digitalWrite(txD2, LOW);
      digitalWrite(txD3, LOW);
      lcd.print("<-----Standby------>");
      break;
    case 1:
      showGPS = true;
      transmit = true;
      digitalWrite(txD0, LOW);//Speed mode
      digitalWrite(txD1, HIGH);
      digitalWrite(txD2, LOW);
      digitalWrite(txD3, LOW);
      lcd.print("<----Speed Mode---->");
      break;
    case 2:
      showGPS = true;
      transmit = true;
      digitalWrite(txD0, LOW);//Step mode
      digitalWrite(txD1, LOW);
      digitalWrite(txD2, HIGH);
      digitalWrite(txD3, LOW);
      lcd.print("<----Step Mode----->");
      break;
    case 3:
      if (gpsCoordsIndex < 0) {
        lcd.print("<---Capture Mode--->");
        showGPS = false;
        transmit = false;
        gpsCoordsIndex++;
      } else if (gpsCoordsIndex > arraySize) {
        EEPROMAdress = EEPROM_writeAnything(EEPROMAdress, gpsCaptureData);
        lcd.print("--------------------");
        lcd.setCursor(0, 2);
        lcd.print("Wrote ");
        lcd.setCursor(6, 2);
        lcd.print(EEPROMAdress);
        lcd.setCursor(10, 2);
        lcd.print("bytes to");
        lcd.setCursor(0, 3);
        lcd.print("EEPROM!");
        gpsCoordsIndex = -1;
      } else {
        gpsCaptureData.gpsCoordsLat [gpsCoordsIndex] = currGpsCoordsLat;
        gpsCaptureData.gpsCoordsLong [gpsCoordsIndex] = currGpsCoordsLong;
        lcd.print("<---Capture [");
        lcd.setCursor(13, 1);
        lcd.print(gpsCoordsIndex);
        lcd.setCursor(15, 1);
        lcd.print("]--->");
        lcd.setCursor(0, 2);
        lcd.print("cLat:  ");
        lcd.print(currGpsCoordsLat);
        lcd.setCursor(0, 3);
        lcd.print("cLong: ");
        lcd.print(currGpsCoordsLong);
        gpsCoordsIndex++;
      }
      break;
    case 4:
      if (gpsCoordsIndex < 0) {
        lcd.print("<---Saved Data---->");
        showGPS = false;
        gpsCoordsIndex++;
      } else if (gpsCoordsIndex == arraySize) {
        lcd.print("<------Done------->");
        gpsCoordsIndex = -1;
      } else {
        lcd.print("<----Saved [");
        lcd.setCursor(12, 1);
        lcd.print(gpsCoordsIndex);
        lcd.setCursor(14, 1);
        lcd.print("]---->");
        lcd.setCursor(0, 2);
        lcd.print("sLat:  ");
        lcd.print(gpsCaptureData.gpsCoordsLat [gpsCoordsIndex]);
        lcd.setCursor(0, 3);
        lcd.print("sLong: ");
        lcd.print(gpsCaptureData.gpsCoordsLong [gpsCoordsIndex]);
        gpsCoordsIndex++;
      }
      break;
    case 5:
      transmit = false;
      showGPS = false;
      lcd.print("<Current Track Pos>");
      showTrackPos();
      break;
    case 6:
      lcd.setCursor(0, 1);
      lcd.print("<Send Current Pos->");
      sendRF = true;
      transmit = false;
      showGPS = false;
      if (setWonky == false) {//toggle wonky
        setWonky = true;
        countWonkyON = 0;
      } else {
        setWonky = false;
        countWonkyOFF = 0;
      }
      if (test < 60) { //Just test code!!!!!!!!!!!!!!!!!!!!
        test = test + 1;
      } else {
        test = 0;
      }
      break;
  }
}

