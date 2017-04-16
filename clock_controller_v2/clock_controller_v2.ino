#include <Stepper.h>
#include <LiquidCrystal.h>
#include <Wire.h>
#include <AccelStepper.h>

//Steppers
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution
const int stepsPerClockRevolution = 200;
// Stepper rightStepper(stepsPerRevolution, 45, 43, 41, 39);
// Stepper leftStepper(stepsPerRevolution, 44, 42, 40, 38);

AccelStepper rightStepper(AccelStepper::FULL4WIRE, 45, 43, 41, 39);
AccelStepper leftStepper(AccelStepper::FULL4WIRE, 44, 42, 40, 38);

const float stepToRev = 0.005; //(1 / stepsPerRevolution) * 200/60; // multiply by step period to get frequency in Herz ( (1 / stepsPerRevolution) * gearRatio)
float gearRatio = 1;
int stepSpeed = 2;
float stepSpeeds [2] = {0.0, 0.0};
bool runLeftStepper = false;
bool runRightStepper = false;
long stepTimer = 0;
int lmStepCount = 0;
int rmStepCount = 0;
String clockPosition = "000000";
int prevPos = 0;
int posList [3] = {0, 0, 0};   // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! MAKE LARGER !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int posListCount = 0;
int rmPos = 0;
int stepDir = -1;
float maxSpeed = 1000.0;

//LCD
LiquidCrystal lcd(47, 49, 46, 48, 50, 52);

//Pushbuttons
const int leftButtonPin = 51;
const int rightButtonPin = 53;
int leftButtonState;
int rightButtonState;
int lastLeftButtonState = LOW;
int lastRightButtonState = LOW;
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

// RF Reciever
int rsPower = 13;
int rsD3 = 12;
int rsD2 = 11;
int rsD1 = 10;
int rsD0 = 9;
int rsD0State = LOW;
int vt = 8;//High when signal recieved
long rfWaitTime = 0;
bool rfON = false;
bool recieveMode = false;
bool recievePos = false;
bool recieved = true;
int recFreq = 2000;//1 sec. Half of transFreq
int countWonkyON = 0;
int countWonkyOFF = 0;
bool setWonky = false;

//Menu
int menuToggle = 0;
int menuSelect = 0;
const int menuSize = 5;

void setup() {
  Serial.begin (115200);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);

  pinMode(rsPower, OUTPUT);
  digitalWrite(rsPower, HIGH);
  pinMode(rsD0, INPUT);
  pinMode(rsD1, INPUT);
  pinMode(rsD2, INPUT);
  pinMode(rsD3, INPUT);

  // set the speed at 60 rpm:
  rightStepper.setMaxSpeed(5);
  leftStepper.setMaxSpeed(5);
  rightStepper.setAcceleration(100.0);
  leftStepper.setAcceleration(100.0);

  doMenuToggle();
  doMenuSelect();

  // Wire.begin(8);                // join i2c bus with address #8
  // Wire.onReceive(receiveEvent);
}

void loop() {
  leftButtonRead();
  rightButtonRead();

  rfRead();
  rfBlink();

  if (setWonky)
    wonkyPos(rmPos, 5);
  else
    doRunSteppers(1);
}

void wonkyPos(int pos, int range) {

  int offset = pos - stepsToPos(lmStepCount, stepsPerClockRevolution);

  leftStepper.setMaxSpeed(stepDir);
  lmStepCount = lmStepCount + stepDir;

  if (offset < -range)
    stepDir = 1;
  else if (offset > range)
    stepDir = -1;
}

void detectWonky(int pos) {
  if (pos == 62) {
    countWonkyON++;
    lcd.setCursor(0, 1);
    lcd.print("far count: ");
    lcd.print(countWonkyON);
  } else {
    countWonkyON = 0;
  }

  if (pos == 63) {
    countWonkyOFF++;
    lcd.setCursor(0, 1);
    lcd.print("ish count: ");
    lcd.print(countWonkyOFF);
  } else {
    countWonkyOFF = 0;
  }

  if (countWonkyOFF > 5) {
    setWonky = false;
    stepDir = -1;
  }

  if (countWonkyON > 5) {
    setWonky = true;
  }
}

int stepsToPos(int stepPos, int steps) {

  return (int)(stepPos - ((int)(stepPos / steps)) * steps);
}

int cyclicDiff(int a1, int a2) {

  return ((a1 - a2) / abs(a1 - a2)) * (30 - abs(abs(a1 - a2) - 30));
}

void moveStepperToPos(int pos) {
  int rmStepPos = stepsToPos(rmStepCount, stepsPerClockRevolution);//Clock hand position

  if (pos == rmStepPos) {
    //Do Nothing!
  } else {
    rightStepper.moveTo(-cyclicDiff(pos, rmStepPos));
    rmStepCount = rmStepCount + cyclicDiff(pos, rmStepPos);
  }
}

void rfRead() {
  // Serial.println("Read MODE");
  if (digitalRead(vt) == HIGH)  {
    // Serial.println("Reading...");
    if (recievePos == true && recieved == false) {// detecting leading edge

      int pos = recBinToDec(clockPosition);// Read position

      if (millis() - stepTimer  > recFreq) { //Only set position after no signal for 10 sec

        clearScreen(1);
        int failCount = 0;

        // detectWonky(pos);

        for (int i = 0; i < 3; i++) {
          if (abs(cyclicDiff(pos, posList[i])) > 3) {// Check if current pos is within range of previous 3 positions. Fails if greater than 3.
            failCount++;
          }

          lcd.print(posList[i]);
          lcd.print(" ");
        }

        if (failCount < 2 && pos < 61) { // Only move stepper if failCount voting wins (Best of 3) and not de/activating wonky
          moveStepperToPos(pos); // Move Stepper
          int rmPos = pos;
          lcd.print("S ");
        } else {
          //FAILED!!
          lcd.print("F ");
        }

        if (posListCount < 2) { // Fill array with previous 3 postions
          posListCount++;
        } else {
          posListCount = 0;
        }
        posList[posListCount] = pos;

        lcd.print(pos);

        clockPosition = "000000";
        stepTimer = millis();
      }
      //lcd.print(pos);
      recieved = true;
    }
  } else {
    recieved = false;
    //No signal coming in!
  }
}

void rfBlink() {

  if (digitalRead(vt) == LOW) {
    lcd.setCursor(14, 0);
    lcd.print("--");
  }
  else if (digitalRead(vt) == HIGH) {
    lcd.setCursor(14, 0);
    lcd.print("rf");
  }
}

int recBinToDec(String prevBin) {

  if (digitalRead(rsD0) == LOW) {// First nibble
    if (digitalRead(rsD1) == HIGH) //Bit 1
      prevBin.setCharAt(0, '1');
    else
      prevBin.setCharAt(0, '0');

    if (digitalRead(rsD2) == HIGH)//Bit 2
      prevBin.setCharAt(1, '1');
    else
      prevBin.setCharAt(1, '0');

    if (digitalRead(rsD3) == HIGH)//Bit 3
      prevBin.setCharAt(2, '1');
    else
      prevBin.setCharAt(2, '0');;
  }

  if (digitalRead(rsD0) == HIGH) {//Second nibble
    if (digitalRead(rsD1) == HIGH)//Bit 4
      prevBin.setCharAt(3, '1');
    else
      prevBin.setCharAt(3, '0');

    if (digitalRead(rsD2) == HIGH)//Bit 5
      prevBin.setCharAt(4, '1');
    else
      prevBin.setCharAt(4, '0');

    if (digitalRead(rsD3) == HIGH)//Bit 6
      prevBin.setCharAt(5, '1');
    else
      prevBin.setCharAt(5, '0');
  }

  clockPosition = prevBin;

  int dec = prevBin.substring(5, 6).toInt()  + prevBin.substring(4, 5).toInt() * 2 + prevBin.substring(3, 4).toInt() * 4 + prevBin.substring(2, 3).toInt() * 8 + prevBin.substring(1, 2).toInt() * 16 + prevBin.substring(0, 1).toInt() * 32; //Convert BIN to DEC

  return dec;
}

void doRunSteppers(int stepSize) {
  // rightStepper.setMaxSpeed(stepSpeed);
  // leftStepper.setMaxSpeed(stepSpeed);
  if (runLeftStepper == true) {
    leftStepper.run();
    // leftStepper.step(stepSize);
    // lmStepCount = lmStepCount + stepSize;
  }

  if (runRightStepper == true) {
    rightStepper.run();
    // rightStepper.step(stepSize);
    // rmStepCount = rmStepCount + stepSize;
  }
}

void leftButtonRead() {

  int reading = digitalRead(leftButtonPin);

  if (reading != lastLeftButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != leftButtonState) {
      leftButtonState = reading;
      if (leftButtonState == LOW) {
        leftButtonState = digitalRead(leftButtonPin);
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

  int reading = digitalRead(rightButtonPin);

  if (reading != lastRightButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (reading != rightButtonState) {
      rightButtonState = reading;
      if (rightButtonState == LOW) {
        rightButtonState = digitalRead(rightButtonPin);

        menuSelect = menuToggle;
        doMenuSelect();
      }
    }
  }
  lastRightButtonState = reading;
}

void doMenuToggle() {
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (menuToggle) {
    case 0:
      lcd.print("Hauptbahnhof");
      lcd.setCursor(0, 1);
      lcd.print("Clock - Standby");
      break;
    case 1:
      lcd.print("---speed mode---");
      break;
    case 2:
      lcd.print("---DMX mode----");
      break;
    case 3:
      lcd.print("--speed+ mode---");
      break;
    case 4:
      lcd.print("--rx sel mode---");
      break;
    case 5:
      lcd.print("--rx pos mode---");
      break;
  }
}

void doMenuSelect () {
  lcd.clear();
  lcd.setCursor(0, 0);
  switch (menuSelect) {
    case 0:
      lcd.print("HAUPTBAHNHOF");
      lcd.setCursor(0, 1);
      lcd.print("CLOCK - STANDBY");
      runLeftStepper = false;
      runRightStepper = false;
      recieveMode = false;
      recievePos = false;
      setWonky = false;
      break;
    case 1:
      lcd.print("---SPEED MODE---");
      lcd.setCursor(0, 1);
      lcd.print("Speed = ");
      lcd.setCursor(8, 1);
      lcd.print(rpmToHz(50)); // revolutions per second
      lcd.setCursor(14, 1);
      lcd.print("Hz");
      rightStepper.setMaxSpeed(50);
      leftStepper.setMaxSpeed(50);
      runLeftStepper = true;
      runRightStepper = true;
      recieveMode = false;
      recievePos = false;
      setWonky = false;
      break;
    case 2:
      lcd.print("---DMX MODE----");
      lcd.setCursor(0, 1);
      lcd.print("Speed = ");
      lcd.setCursor(8, 1);
      lcd.print(rpmToHz(stepSpeed)); // revolutions per second
      lcd.setCursor(14, 1);
      lcd.print("Hz");
        //I2C on address 8 - Slave
      Wire.begin(8);                // join i2c bus with address #8
      Wire.onReceive(receiveEvent);
      rightStepper.setMaxSpeed(stepSpeed);
      leftStepper.setMaxSpeed(stepSpeed);
      rightStepper.moveTo(1000000000000);
      leftStepper.moveTo(1000000000000);
      runLeftStepper = true;
      runRightStepper = true;
      recieveMode = false;
      recievePos = false;
      setWonky = false;
      break;
    case 3:
      lcd.print("--SPEED+ MODE---");
      if (runRightStepper == false) {
        stepSpeed = 2;
      } else {
        stepSpeed = stepSpeed + 5;
      }
      rightStepper.setMaxSpeed(stepSpeed); 
      leftStepper.setMaxSpeed(stepSpeed);

      lcd.setCursor(0, 1);
      lcd.print("Speed = ");
      lcd.setCursor(8, 1);
      lcd.print(rpmToHz(stepSpeed)); // revolutions per second
      lcd.setCursor(14, 1);
      lcd.print("Hz");

      runLeftStepper = true;
      runRightStepper = true;
      recieveMode = false;
      recievePos = false;
      setWonky = false;
      break;
    case 4:
      lcd.print("--RX SEL MODE---");
      runLeftStepper = false;
      runRightStepper = false;
      recieveMode = true;
      recievePos = false;
      setWonky = false;
      break;
    case 5:
      lcd.print("--RX POS MODE---");
      leftStepper.setMaxSpeed(2);
      runLeftStepper = false;
      runRightStepper = false;
      recieveMode = false;
      recievePos = true;
      setWonky = false;
      rightStepper.setMaxSpeed(2);
      leftStepper.setMaxSpeed(2);
      break;
  }
}

float rpmToHz(long rpm) {
  return (float)(rpm * (3.33) / 60);
}

void clearScreen(int row) {

  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
}

void requestEvent() {
  // Serial.println("send");
  // Wire.write(cm, SONAR_NUM);                                                        // respond with message of SONAR_NUM bytes
  // for (uint8_t i = 0; i < SONAR_NUM; i++) cm[i] = 0;                                // Make distance zero in case there's no ping echo for this sensor.
}

void receiveEvent(int howMany) {   // USE HOW MANY !!!!!!!!!!!!!!
  int count = 0;
  bool changeSpeed = false;
  while (0 < Wire.available()) { // loop through all but the last
    // if(count < 2) stepSpeeds[count] = Wire.read(); // receive byte as a character
    float speed = Wire.read();
    if(speed == stepSpeeds[count]) changeSpeed = true;
    stepSpeeds[count] = speed;
    count++;
  }
  
  if(count == 2 && changeSpeed == true){
    rightStepper.setMaxSpeed(stepSpeeds[0] * (maxSpeed / 255.0));
    leftStepper.setMaxSpeed(stepSpeeds[1] * (maxSpeed / 255.0));
    rightStepper.moveTo(1000000000000);
    leftStepper.moveTo(1000000000000);

    lcd.setCursor(8, 1);
    lcd.print(stepSpeeds[0] * (maxSpeed / 255.0)); // revolutions per second
  }
  count = 0;
  
  // stepSpeed = Wire.read();
  // while (1 < Wire.available()) { // loop through all but the last
  //   int x = Wire.read() * 4; // receive byte as a character
  //   Serial.print(c);         // print the character
  // }
  // int x = Wire.read();    // receive byte as an integer

  // if(stepSpeed.size() != 2){
  //   stepSpeeds.clear();
  //   stepSpeeds.push_back(0);
  //   stepSpeeds.push_back(0);
  //   Serial.println("Not enough data from DMX! Set stepSpeeds to 0");
  // }
}