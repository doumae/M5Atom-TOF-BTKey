#include <M5Atom.h>
#include <Adafruit_VL53L0X.h>
#include <BleKeyboard.h>

#define STATE_INIT    0
#define STATE_FAR     1
#define STATE_APPROCH 2
#define STATE_CLOSE   3
#define STATE_AWAY    4

#define LED_OFF       0x000000
#define LED_INIT      0x110000  // RED
#define LED_FAR       0x0000FF  // BLUE
#define LED_APPROCH   0xFF00FF
#define LED_CLOSE     0x00FF00  // GREEN
#define LED_AWAY      0xFFFF00

#define STATE_HOLD    5           // TICK * HOLD times
#define THRESHOLD_DISTANCE  1500  // mm
#define TICK_MS       100         // ms

#define BLEKEY_DEVICENAME "M5Atom KBD 1"
#define BLEKEY_ON   '1'
#define BLEKEY_OFF  '0'

Adafruit_VL53L0X lox;
BleKeyboard bleKeyboard(BLEKEY_DEVICENAME);

int state;
int state_count;
int last_millis;

int blink_flag;

void led(unsigned int color) {
  M5.dis.drawpix(0, color);
}

void key(char key){
  if(bleKeyboard.isConnected()){
    bleKeyboard.write(key);
  }
}

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

  bleKeyboard.begin();
  Serial.println("Start BLE Keyboard");


  state = STATE_INIT;
  state_count = 0;
  blink_flag = false;

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
          key(BLEKEY_ON);

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
          key(BLEKEY_OFF);

        }

      }
      break;

  }

  if (blink_flag) {
    switch (state) {
      case STATE_FAR:
        led(LED_FAR);
        break;
      case STATE_APPROCH:
        led(LED_APPROCH);
        break;
      case STATE_CLOSE:
        led(LED_CLOSE);
        break;
      case STATE_AWAY:
        led(LED_AWAY);
        break;
      default:
        led(LED_INIT);
        break;
    }
  } else {
    led(LED_OFF);
  }
  blink_flag = ! blink_flag;

  while ((millis() - last_millis) < TICK_MS) {
    delay(10);
  }
  last_millis = millis();

}
