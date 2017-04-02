/*
********************************************
14CORE ULTRASONIC DISTANCE SENSOR CODE TEST
********************************************
*/
#include <Wire.h>  

#define IO_ADDR (0x38)

#define S0 16
#define S1 0
#define S2 2
#define S3 14

#define Z 12

//#define S0 5
//#define S1 7
//#define S2 6
//#define S3 1
 
// holds incoming values from 74HC4067                  
byte muxValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
int toggle = 0;
 
// NodeMCU Pin D1 > TRIGGER | Pin D2 > ECHO
 
void setup() {
  
  Serial.begin (9600);
  // pinMode(TRIGGER, OUTPUT);
  // pinMode(ECHO, INPUT);
  // pinMode(BUILTIN_LED, OUTPUT);
    
  pinMode(S0,OUTPUT);
  pinMode(S1,OUTPUT);
  pinMode(S2,OUTPUT);
  pinMode(S3,OUTPUT);
  pinMode(Z,OUTPUT);
  digitalWrite(Z, HIGH);

  
  Wire.begin(4,5);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
}
 
void loop() {

  delay(200);
}

void requestEvent() {
  long duration;
  int distance; // mm

  for(int i = 0; i < 1; i++){
      pinMode(Z,OUTPUT);
      digitalWrite(Z, LOW);
      setPin(i);
      delayMicroseconds(2); 
      
      digitalWrite(Z, LOW);
      delayMicroseconds(10); 
      
      digitalWrite(Z, LOW);
      pinMode(Z,INPUT);
      setPin(i + 8);
      duration = pulseIn(Z, HIGH);
      distance = (duration/2) / 2.91;
      
      Serial.println(distance);    
  }


  Wire.write(distance); // respond with message of 6 bytes
}

// function to select pin on 74HC4067
void setPin(int outputPin) {
  switch(outputPin) {
    case 0 :
      Serial.println("Pin 0");  
      digitalWrite(S0, LOW);
      digitalWrite(S1, LOW);
      digitalWrite(S2, LOW);
      digitalWrite(S3, LOW);
      break;
    case 1 :
      Serial.println("Pin 1");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, LOW);
      digitalWrite(S2, LOW);
      digitalWrite(S3, LOW);
      break;
    case 2 :
      Serial.println("Pin 2");
      digitalWrite(S0, LOW);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, LOW);
      digitalWrite(S3, LOW);
      break;
    case 3 :
      Serial.println("Pin 3");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, LOW);
      digitalWrite(S3, LOW);
      break;
    case 4 :
      Serial.println("Pin 4");
      digitalWrite(S0, LOW);
      digitalWrite(S1, LOW);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, LOW);
      break;
    case 5 :
      Serial.println("Pin 5");  
      digitalWrite(S0, HIGH);
      digitalWrite(S1, LOW);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, LOW);
      break;
    case 6 :
      Serial.println("Pin 6");
      digitalWrite(S0, LOW);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, LOW);
      break;
    case 7 :
      Serial.println("Pin 7");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, LOW);
      break;
    case 8 :
      Serial.println("Pin 8");
      digitalWrite(S0, LOW);
      digitalWrite(S1, LOW);
      digitalWrite(S2, LOW);
      digitalWrite(S3, HIGH);
      break;
    case 9 :
      Serial.println("Pin 9");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, LOW);
      digitalWrite(S2, LOW);
      digitalWrite(S3, HIGH);
      break;
    case 10 :
      Serial.println("Pin 10");
      digitalWrite(S0, LOW);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, LOW);
      digitalWrite(S3, HIGH);
      break;
    case 11 :
      Serial.println("Pin 11");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, LOW);
      digitalWrite(S3, HIGH);
      break;
    case 12 :
      Serial.println("Pin 12");
      digitalWrite(S0, LOW);
      digitalWrite(S1, LOW);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, HIGH);
      break;
    case 13 :
      Serial.println("Pin 13");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, LOW);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, HIGH);
      break;
    case 14 :
      Serial.println("Pin 14");
      digitalWrite(S0, LOW);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, HIGH);
      break;
    case 15 :
      Serial.println("Pin 15");
      digitalWrite(S0, HIGH);
      digitalWrite(S1, HIGH);
      digitalWrite(S2, HIGH);
      digitalWrite(S3, HIGH);
      break;
   }
}
