#include <FastLED.h>

#define DEBUG false

// Distance constants
#define PIN_TRIG           3
#define PIN_ECHO           2
#define DIST_MIN           6
#define DIST_MAX           200
#define DIST_AVG           5

// LED strip constants
#define LED_PIN            5
#define NUM_LEDS           30
#define BRIGHTNESS         64
#define LED_TYPE           WS2812B
#define COLOR_ORDER        GRB
#define UPDATES_PER_SECOND 100
CRGB leds[NUM_LEDS];

// Globals
long lastDistance = DIST_MAX + 1;
CRGBPalette16 currentPalette = RainbowColors_p;
TBlendType    currentBlending = NOBLEND;

long distances[DIST_AVG];
long distances_idx = 0;
bool distances_init = false;
long lastaverage = 0;
long outOfRangeCount = 0;

void setup() {
  delay(3000);   // A power-up safety delay for the LEDs
  Serial.begin(9600);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(currentPalette, 16, CRGB::White);
  currentPalette[1] = CRGB::Black;
}

long getDistance() {
  long duration, distance;
  // Make sure the trigger is off
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  // Trip the trigger
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Wait for the data to come in from the echo pin
  duration = pulseIn(PIN_ECHO, HIGH);
  // See http://howtomechatronics.com/tutorials/arduino/ultrasonic-sensor-hc-sr04/
  return (duration * 0.034) / 2;
}

bool inRange(long distance) {
  return distance >= 0 && distance <= 200;
}

void setLEDs(long distance) {
  uint8_t brightness = 255;
  
  long lighted = 0;
  if (inRange(distance)) {
    float pct = ((float)distance - 4) / (200 - 4);
    // Get the inverted range to light up
    lighted = NUM_LEDS - (long)(pct * NUM_LEDS);
    if (DEBUG) {
      Serial.print("Pct: ");
      Serial.print(pct);
      Serial.print("; Lighting ");
      Serial.print(lighted);
      Serial.println(" LEDs");
    }
  }

  for (int i=0; i < lighted; i++) {
    leds[i] = CRGB::White;
  }
  for (int i=lighted; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}

void fillDistanceArray(long value) {
  for (int i=0; i < DIST_AVG; i++) {
    distances[i] = value;
  }
  lastaverage = value;
}

long normalizeDistance(long distance) {
  if (inRange(distance)) {
    outOfRangeCount = 0;
    if (!distances_init) {
      fillDistanceArray(distance);
      distances_init = true;
    } else {
      distances[distances_idx] = distance;
      distances_idx = (distances_idx+1) % DIST_AVG;
    }
  } else {
    if (++outOfRangeCount > DIST_AVG) {
      fillDistanceArray(distance);
    }
  }

  // Smooths out some erratic behavior in echo sensor
  long ret = 0;
  long counted = 0;
  long realaverage = 0;

  for (int i=0; i < DIST_AVG; i++) {
    realaverage += distances[i];
    // If this value is more than 20% higher/lower than the last average, don't count it
    if (min(distances[i], lastaverage) / max(distances[i], lastaverage) * 100 > 20) {
      continue;
    }
    ret += distances[i];
    counted++;
  }
  long avg = (long)((float)ret / counted);
  // In order to maintain a floating average, use the real average rather than "counted" average
  lastaverage = (long)((float)realaverage / DIST_AVG);
  
  Serial.print("Data: [");
  for (int i=0; i < DIST_AVG; i++) {
    Serial.print(distances[i]);
    Serial.print(",");
  }
  Serial.print("]; avg=");
  Serial.print(avg);
  Serial.print("; lastavg=");
  Serial.println(lastaverage);

  // average
  return avg;
}

void loop() {
  long distance = getDistance();
  distance = normalizeDistance(distance);
  setLEDs(distance);
  Serial.print(distance);
  Serial.println(" cm (2)");
  delay(175);
}
