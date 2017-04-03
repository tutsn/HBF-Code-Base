#include <NewPing.h>
#include <Wire.h>

#define SONAR_NUM     2 // Number of sensors.
#define MAX_DISTANCE 100 // Maximum distance (in cm) to ping.
#define PING_INTERVAL 33 // Milliseconds between sensor pings (29ms is about the min to avoid cross-sensor echo).

unsigned long pingTimer[SONAR_NUM]; // Holds the times when the next ping should happen for each sensor.
byte cm[SONAR_NUM];         // Where the ping distances are stored.
uint8_t currentSensor = 0;          // Keeps track of which sensor is active.

NewPing sonar[SONAR_NUM] = {     // Sensor object array.
  NewPing(5, 6, MAX_DISTANCE), // Each sensor's trigger pin, echo pin, and max distance to ping.
  NewPing(35, 33, MAX_DISTANCE)
};

int smoothing = 5;

int gnPins[] = {
  31, 7
};

int v5Pins[] = {
  37, 4
};

void setup() {
  
  Serial.begin (115200);
    
  for (int i = 0; i < SONAR_NUM; i++) {
    pinMode(gnPins[i],OUTPUT);
    pinMode(v5Pins[i],OUTPUT);
    digitalWrite(gnPins[i], LOW);
    digitalWrite(v5Pins[i], HIGH);
  }

  pingTimer[0] = millis() + 75;           // First ping starts at 75ms, gives time for the Arduino to chill before starting.
  for (uint8_t i = 1; i < SONAR_NUM; i++) // Set the starting time for each sensor.
    pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  

  //I2C on address 8 - Slave
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onRequest(requestEvent); // register event
  Wire.onReceive(receiveEvent);
}
 
void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) {                           // Loop through all the sensors.
    if (millis() >= pingTimer[i]) {                                   // Is it this sensor's time to ping?
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;                      // Set next time this sensor will be pinged.
      sonar[currentSensor].timer_stop();                              // Make sure previous timer is canceled before starting a new ping (insurance).
      currentSensor = i;                                              // Sensor being accessed.
      sonar[currentSensor].ping_timer(echoCheck);                     // Do the ping (processing continues, interrupt will call echoCheck to look for echo).
      sonar[currentSensor].ping_median(smoothing);
    }
  }
  Serial.println(cm[1]);
}

void echoCheck() { // If ping received, set the sensor distance to array.
  if (sonar[currentSensor].check_timer())
    cm[currentSensor] = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM;
}

void requestEvent() {
  Serial.println("send");
  Wire.write(cm, SONAR_NUM);                                                        // respond with message of SONAR_NUM bytes
  // for (uint8_t i = 0; i < SONAR_NUM; i++) cm[i] = 0;                                // Make distance zero in case there's no ping echo for this sensor.
}

void receiveEvent(int howMany) {
  smoothing = Wire.read();
}
