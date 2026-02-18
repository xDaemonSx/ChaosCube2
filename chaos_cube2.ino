#include <FastLED.h>

/***************************************************
 * Cube #2 – Chaotic Cube (FINAL)
 ***************************************************/

#define DEBUG 1

// ================= LED CONFIG =================
#define LED_PIN     6
#define NUM_LEDS    32
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS  120

CRGB leds[NUM_LEDS];

// ================= TOUCH =================
#define TOUCH_PIN 2
const unsigned long TOUCH_COOLDOWN = 300;

// ================= TIMING =================
const unsigned long AUTO_OFF_TIME = 10000;

// ================= FSM =================
enum State {
  OFF,
  EFFECT_DELAY,
  EFFECT_BLINK,
  EFFECT_STRANGE,
  EFFECT_FADE,
  EFFECT_FLICKER,
  EFFECT_RAINBOW   // глобальный цвет с плавной сменой
};

State currentState = OFF;

// ================= GLOBAL =================
bool cubeOn = false;
bool lastTouchState = LOW;

unsigned long lastTouchTime  = 0;
unsigned long cubeOnTime     = 0;
unsigned long lastStepTime   = 0;

// ================= EFFECT VARS =================
uint8_t fadeValue = 0;
bool blinkState = false;
uint8_t globalHue = 0;   // ← главный параметр цвета

// ================= LOGGING =================
#if DEBUG
void logMsg(const char* msg) {
  Serial.print("[");
  Serial.print(millis());
  Serial.print(" ms] ");
  Serial.println(msg);
}
#else
void logMsg(const char* msg) {}
#endif

// ================= SETUP =================
void setup() {
  pinMode(TOUCH_PIN, INPUT);

#if DEBUG
  Serial.begin(115200);
  Serial.println(F("=== Cube #2 FINAL GLOBAL COLOR FADE START ==="));
#endif

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear();
  FastLED.show();

  randomSeed(analogRead(A0));
}

// ================= LOOP =================
void loop() {
  handleTouch();
  runFSM();
}

// ================= TOUCH HANDLER =================
void handleTouch() {
  bool touchState = digitalRead(TOUCH_PIN);
  unsigned long now = millis();

  if (touchState == HIGH &&
      lastTouchState == LOW &&
      now - lastTouchTime > TOUCH_COOLDOWN) {

    lastTouchTime = now;
    cubeOn = !cubeOn;

    if (cubeOn) {
      cubeOnTime = now;
      logMsg("Touch → CUBE ON");
      startRandomEffect();
    } else {
      logMsg("Touch → CUBE OFF (manual)");
      turnOff();
    }
  }

  lastTouchState = touchState;
}

// ================= START RANDOM EFFECT =================
void startRandomEffect() {
  int r = random(1, 7);
  lastStepTime = millis();
  fadeValue = 0;
  blinkState = false;
  globalHue = random(0, 255);

  switch (r) {
    case 1:
      logMsg("Effect: DELAY + SOLID");
      currentState = EFFECT_DELAY;
      break;

    case 2:
      logMsg("Effect: BLINK");
      currentState = EFFECT_BLINK;
      break;

    case 3:
      logMsg("Effect: STRANGE (BREATH)");
      currentState = EFFECT_STRANGE;
      break;

    case 4:
      logMsg("Effect: FADE LOOP");
      currentState = EFFECT_FADE;
      break;

    case 5:
      logMsg("Effect: FLICKER");
      currentState = EFFECT_FLICKER;
      break;

    case 6:
      logMsg("Effect: GLOBAL COLOR FADE");
      currentState = EFFECT_RAINBOW;
      break;
  }
}

// ================= TURN OFF =================
void turnOff() {
  currentState = OFF;
  FastLED.clear();
  FastLED.show();
  logMsg("State → OFF");
}

// ================= FSM CORE =================
void runFSM() {
  unsigned long now = millis();

  // -------- AUTO OFF --------
  if (cubeOn && now - cubeOnTime >= AUTO_OFF_TIME) {
    logMsg("AUTO OFF (10s elapsed)");
    cubeOn = false;
    turnOff();
    return;
  }

  switch (currentState) {

    case OFF:
      break;

    case EFFECT_DELAY:
      if (now - lastStepTime > 1500) {
        fill_solid(leds, NUM_LEDS, CRGB::White);
        FastLED.show();
      }
      break;

    case EFFECT_BLINK:
      if (now - lastStepTime > 400) {
        lastStepTime = now;
        blinkState = !blinkState;
        if (blinkState) {
          fill_solid(leds, NUM_LEDS, CRGB::White);
        } else {
          FastLED.clear();
        }
        FastLED.show();
      }
      break;

    case EFFECT_STRANGE:
      if (now - lastStepTime > 30) {
        lastStepTime = now;
        uint8_t v = beatsin8(6, 40, 180);
        fill_solid(leds, NUM_LEDS, CRGB(v, v, v));
        FastLED.show();
      }
      break;

    case EFFECT_FADE:
      if (now - lastStepTime > 20) {
        lastStepTime = now;
        fadeValue += 3;
        if (fadeValue > 255) fadeValue = 0;
        fill_solid(leds, NUM_LEDS, CHSV(0, 0, fadeValue));
        FastLED.show();
      }
      break;

    case EFFECT_FLICKER:
      if (now - lastStepTime > 80) {
        lastStepTime = now;
        FastLED.clear();
        for (int i = 0; i < random(1, 6); i++) {
          leds[random(NUM_LEDS)] = CRGB::White;
        }
        FastLED.show();
      }
      break;

    case EFFECT_RAINBOW:
      // 🌈 ОДИН ЦВЕТ НА ВСЕХ LED, ПЛАВНАЯ СМЕНА
      if (now - lastStepTime > 40) {
        lastStepTime = now;
        globalHue++; // плавный переход оттенка
        fill_solid(leds, NUM_LEDS, CHSV(globalHue, 255, 200));
        FastLED.show();
      }
      break;
  }
}
