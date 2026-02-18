#include <Adafruit_NeoPixel.h>

const int SENSOR_PIN = 2;
const int LED_PIN = 9;
const int NUM_LEDS = 32;

// --- НАСТРОЙКИ ---
unsigned long workTime = 10000; // 10 секунд
// -----------------

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

bool isActive = false;
unsigned long startTime = 0;
bool lastSensorState = LOW;

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
  strip.begin();
  strip.setBrightness(100); // Яркость выше, так как это "стабильный" куб
  strip.show();
  Serial.println("Cube #1: Order Mode. 32 LEDs Static Timer.");
}

void setAll(uint32_t color) {
  for(int i = 0; i < NUM_LEDS; i++) strip.setPixelColor(i, color);
  strip.show();
}

void loop() {
  bool current = digitalRead(SENSOR_PIN);

  // Включение по фронту касания
  if (current == HIGH && lastSensorState == LOW) {
    isActive = true;
    startTime = millis();
    setAll(strip.Color(255, 255, 255)); // Чистый белый
    Serial.println("Cube #1: ON (White). Timer started.");
    delay(200); 
  }
  lastSensorState = current;

  // Автовыключение через 10 секунд
  if (isActive && (millis() - startTime >= workTime)) {
    setAll(strip.Color(0, 0, 0));
    isActive = false;
    Serial.println("Cube #1: OFF (Time is up).");
  }
}