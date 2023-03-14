#include <M5Atom.h>
#include <Adafruit_VL53L0X.h>

#define STATE_INIT    0
#define STATE_FAR     1
#define STATE_APPROCH 2
#define STATE_CLOSE   3
#define STATE_AWAY    4

#define STATE_HOLD    3           // TICK * HOLD times
#define THRESHOLD_DISTANCE  1500  // mm
#define TICK_MS       100         // ms



Adafruit_VL53L0X lox = Adafruit_VL53L0X();

int state;
int state_count;
int last_millis;

int led_flag;

void setup() {

  M5.begin(true, false, true);
  Wire.begin(26, 32);
  Serial.begin(115200);

  Serial.println("Init M5Atom");

  if (!lox.begin()) {
    Serial.println("VL53L0X Error");
  }
  lox.configSensor(Adafruit_VL53L0X::VL53L0X_SENSE_LONG_RANGE);
  lox.startRangeContinuous(100);
  Serial.println("Init VL53L0X");


  state = STATE_INIT;
  state_count = 0;

  last_millis = millis();


}

void loop() {
  

  int sensor_distance;
  while (! lox.isRangeComplete()) {
    delay(10);
  }
  sensor_distance = lox.readRange();

/*
  Serial.print("State: ");
  Serial.print(state);
  Serial.print(" Distance: ");
  Serial.println(sensor_distance);
*/

  switch (state) {

    case STATE_INIT:
      state = STATE_FAR;
      state_count = 0;
      break;

    case STATE_FAR:

      if (sensor_distance < THRESHOLD_DISTANCE) {

        Serial.println("STATE ===> APPROCH");
        state = STATE_APPROCH;
        state_count = STATE_HOLD;

      }
      break;

    case STATE_APPROCH:

      if (sensor_distance > THRESHOLD_DISTANCE) {

        Serial.println("STATE ===> FAR");
        state = STATE_FAR;
        state_count = 0;

      } else {

        state_count --;
        if (state_count < 1) {

          Serial.println("STATE ===> CLOSE");
          state = STATE_CLOSE;
          state_count = 0;

          // SEND KEY
          Serial.println("SEND KEY ON");

        }
      }
      break;

    case STATE_CLOSE:

      if (sensor_distance > THRESHOLD_DISTANCE) {

        Serial.println("STATE ===> AWAY");
        state = STATE_AWAY;
        state_count = STATE_HOLD;

      }
      break;

    case STATE_AWAY:

      if (sensor_distance < THRESHOLD_DISTANCE) {

        Serial.println("STATE ===> CLOSE");
        state = STATE_CLOSE;
        state_count = 0;

      } else {

        state_count --;
        if (state_count < 1) {

          Serial.println("STATE ===> FAR");
          state = STATE_FAR;
          state_count = 0;

          // SEND KEY
          Serial.println("SEND KEY OFF");

        }

      }
      break;

  }



  while ((millis() - last_millis) < TICK_MS) {
    delay(10);
  }
  last_millis = millis();

}
