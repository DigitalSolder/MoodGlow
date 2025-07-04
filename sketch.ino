#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <EEPROM.h>

// Pin definitions optimized for Wokwi
#define LDR_PIN      34
#define TEMP_PIN     35
#define BUTTON_PIN   16
#define BUZZER_PIN   14
#define NEOPIXEL_PIN 4
#define NUM_LEDS     8
#define EEPROM_SIZE  64 // Increased EEPROM size slightly for future expansion if needed

// Enhanced mood system with more detailed states
struct Mood {
  String name;
  String quote;
  uint32_t color; // RGB color
  const int* melody;
  int melodyLength;
  bool requiresMotion;
  float minTemp;
  float maxTemp;
  int lightThreshold; // LDR value threshold
};

// Melody arrays (frequencies in Hz)
const int happyMelody[] = {880, 988, 1047, 0};
const int sadMelody[] = {392, 370, 349, 0};
const int angryMelody[] = {1000, 1000, 800, 0};
const int curiousMelody[] = {784, 880, 988, 0};
const int excitedMelody[] = {1318, 1397, 1480, 0};
const int sleepyMelody[] = {262, 220, 196, 0};
const int calmMelody[] = {329, 294, 262, 0};
const int anxiousMelody[] = {880, 880, 440, 0};
const int gratefulMelody[] = {659, 784, 880, 0};
const int focusedMelody[] = {987, 1047, 1175, 0};
const int relaxedMelody[] = {329, 294, 262, 0};
const int energeticMelody[] = {1760, 1760, 1760, 0};

// Initialize moods with more parameters for smarter detection
Mood moods[] = {
  {"Happy", "Keep Smiling!", Adafruit_NeoPixel::Color(255, 50, 0), happyMelody, 3, true, 20.0, 35.0, 2000},
  {"Sad", "It's okay.", Adafruit_NeoPixel::Color(0, 0, 255), sadMelody, 3, false, 10.0, 25.0, 1000},
  {"Angry", "Take a deep breath.", Adafruit_NeoPixel::Color(255, 0, 0), angryMelody, 3, true, 15.0, 40.0, 1500},
  {"Curious", "Explore the unknown.", Adafruit_NeoPixel::Color(180, 0, 255), curiousMelody, 3, true, 18.0, 30.0, 2500},
  {"Excited", "Feel the energy!", Adafruit_NeoPixel::Color(255, 105, 0), excitedMelody, 3, true, 22.0, 38.0, 3000},
  {"Sleepy", "Time to rest.", Adafruit_NeoPixel::Color(100, 100, 255), sleepyMelody, 3, false, 10.0, 25.0, 500},
  {"Calm", "Stay peaceful.", Adafruit_NeoPixel::Color(255, 255, 100), calmMelody, 3, false, 18.0, 28.0, 1500},
  {"Anxious", "You're doing great.", Adafruit_NeoPixel::Color(255, 255, 0), anxiousMelody, 3, true, 15.0, 32.0, 3500},
  {"Grateful", "Appreciated.", Adafruit_NeoPixel::Color(255, 20, 147), gratefulMelody, 3, false, 15.0, 30.0, 2000},
  {"Focused", "Stay on track.", Adafruit_NeoPixel::Color(255, 255, 255), focusedMelody, 3, false, 18.0, 28.0, 1800},
  {"Relaxed", "Just breathe.", Adafruit_NeoPixel::Color(135, 206, 235), relaxedMelody, 3, false, 16.0, 26.0, 1200},
  {"Energetic", "Channel your power!", Adafruit_NeoPixel::Color(255, 0, 255), energeticMelody, 3, true, 22.0, 40.0, 2800}
};

Adafruit_NeoPixel ring(NUM_LEDS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
// OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins normally)
// The -1 indicates no reset pin.
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MPU6050 mpu;

int currentMoodIndex = 0;
bool buttonState = HIGH;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
bool showIntro = true;

// Timers for periodic events
unsigned long lastSensorUpdate = 0;
const long sensorInterval = 1000; // Update sensors every 1 second

unsigned long lastMoodDetection = 0;
const long moodDetectionInterval = 10000; // Detect mood every 10 seconds

unsigned long lastMelodyPlay = 0;
const long melodyPlayInterval = 5000; // Play melody every 5 seconds

float temperature = 20.0; // Default temperature
int lightVal = 2000;      // Default light value
float accelX = 0, accelY = 0, accelZ = 0; // Accelerometer readings

// Function prototypes
void readSensors();
void handleButtonPress();
int detectMoodFromSensors();
void reactToMood(unsigned long currentMillis); // Modified: Added currentMillis parameter
void updateDisplay();
void showIntroAnimation();
void showColor(uint32_t color);
void flashColor(uint32_t color, int flashes);
void animateFade(uint32_t color);
void animateWave(uint32_t color);
void animateStrobe(uint32_t color);
void animatePulse(uint32_t color);
void animateRainbow();
void playMelody(const int tones[], int length);
void playClickSound();


void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor
  pinMode(BUZZER_PIN, OUTPUT);
  ring.begin(); // Initialize NeoPixel ring
  ring.show();  // Turn all LEDs off

  EEPROM.begin(EEPROM_SIZE);
  // Read saved mood index from EEPROM
  currentMoodIndex = EEPROM.read(0);
  // Validate the read index to prevent out-of-bounds access
  if (currentMoodIndex >= sizeof(moods) / sizeof(moods[0]) || currentMoodIndex < 0) {
    currentMoodIndex = 0; // Default to Happy if invalid
    EEPROM.write(0, currentMoodIndex); // Save default
    EEPROM.commit(); // Commit changes to EEPROM
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display(); // Clear and show empty display

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println(F("Failed to find MPU6050 chip"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println(F("MPU6050 Found!"));
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // Set accelerometer range
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);     // Set gyro range
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);  // Set digital low pass filter bandwidth

  // Set default text properties for OLED
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  // Show intro animation only once at startup
  if (showIntro) {
    showIntroAnimation();
    showIntro = false;
  }

  unsigned long currentMillis = millis(); // Get current time

  // Read sensors at a controlled interval
  if (currentMillis - lastSensorUpdate >= sensorInterval) {
    readSensors();
    lastSensorUpdate = currentMillis; // Update last sensor update time
  }

  // Handle button press with debounce
  handleButtonPress();

  // Auto-detect mood based on sensor data at a controlled interval
  if (currentMillis - lastMoodDetection >= moodDetectionInterval) {
    int detectedMood = detectMoodFromSensors();
    if (detectedMood != currentMoodIndex) { // Only update if mood changed
      currentMoodIndex = detectedMood;
      EEPROM.write(0, currentMoodIndex); // Save new mood to EEPROM
      EEPROM.commit(); // Commit changes
      Serial.print("Mood changed to: ");
      Serial.println(moods[currentMoodIndex].name);
    }
    lastMoodDetection = currentMillis; // Update last mood detection time
  }

  // React to current mood (visuals and audio)
  reactToMood(currentMillis); // Modified: Pass currentMillis to reactToMood()

  // Update display
  updateDisplay();

  // A small delay to prevent the loop from running too fast,
  // but not blocking critical operations.
  delay(10);
}

void readSensors() {
  // Read light sensor (analog value 0-4095 for ESP32 ADC)
  lightVal = analogRead(LDR_PIN);

  // Read temperature (simulated in Wokwi, assuming LM35-like behavior)
  int tempVal = analogRead(TEMP_PIN);
  // Convert analog reading to voltage (assuming 3.3V AREF)
  float voltage = tempVal * 3.3 / 4095.0;
  // Convert voltage to temperature (for LM35: 10mV per degree C, plus 0.5V offset)
  temperature = (voltage - 0.5) * 100.0;
  Serial.print("Temperature: "); Serial.print(temperature); Serial.println(" C");
  Serial.print("Light: "); Serial.println(lightVal);

  // Read accelerometer data
  sensors_event_t a, g, temp_mpu; // temp_mpu is for internal MPU temperature, not used for ambient
  mpu.getEvent(&a, &g, &temp_mpu);
  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;
  Serial.print("Accel: X="); Serial.print(accelX);
  Serial.print(" Y="); Serial.print(accelY);
  Serial.print(" Z="); Serial.println(accelZ);
}

void handleButtonPress() {
  // Read the current state of the button
  buttonState = digitalRead(BUTTON_PIN);

  // If the button state has changed, reset the debounce timer
  if (buttonState != lastButtonState) {
    lastDebounceTime = millis();
  }

  // If the debounce time has passed and the button is pressed (LOW)
  // and it was previously released (HIGH), then it's a valid press.
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Cycle to the next mood
      currentMoodIndex = (currentMoodIndex + 1) % (sizeof(moods) / sizeof(moods[0]));
      EEPROM.write(0, currentMoodIndex); // Save new mood to EEPROM
      EEPROM.commit(); // Commit changes
      playClickSound(); // Play a short sound for feedback
      Serial.print("Button pressed. New mood: ");
      Serial.println(moods[currentMoodIndex].name);
    }
  }
  // Save the current button state for the next loop iteration
  lastButtonState = buttonState;
}

int detectMoodFromSensors() {
  // Enhanced mood detection algorithm
  // Calculate total motion magnitude
  float motion = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
  Serial.print("Calculated Motion: "); Serial.println(motion);

  int bestMatchIndex = currentMoodIndex; // Start with current mood as default best match
  int bestMatchScore = 0; // Score for matching criteria

  for (int i = 0; i < sizeof(moods) / sizeof(moods[0]); i++) {
    int currentScore = 0;

    // Check temperature match
    if (temperature >= moods[i].minTemp && temperature <= moods[i].maxTemp) {
      currentScore += 1;
    }

    // Check motion match
    // If mood requires motion, check if motion is above a threshold (e.g., 1.5 m/s^2, adjust as needed)
    // If mood does NOT require motion, it matches regardless of motion.
    if ((moods[i].requiresMotion && motion > 1.5) || (!moods[i].requiresMotion)) {
      currentScore += 1;
    }

    // Check light match
    // If mood has a light threshold, check if light is above it.
    // If lightThreshold is 0, it means light is not a factor for this mood.
    if (lightVal >= moods[i].lightThreshold || moods[i].lightThreshold == 0) {
      currentScore += 1;
    }

    // If this mood has a higher score, it's a better match
    // Or if scores are equal, prefer the current mood to avoid rapid switching
    if (currentScore > bestMatchScore) {
      bestMatchScore = currentScore;
      bestMatchIndex = i;
    }
  }

  // If no mood matches perfectly (score < 3), or if the current mood is still the best,
  // stick with the current mood. This prevents constant mood changes with minor sensor fluctuations.
  // This logic can be further refined for more nuanced transitions.
  return bestMatchIndex;
}

// Modified: Added currentMillis parameter to reactToMood()
void reactToMood(unsigned long currentMillis) {
  Mood currentMood = moods[currentMoodIndex];

  // Set brightness based on ambient light
  // Map LDR value (0-4095) to NeoPixel brightness (0-255)
  // Ensure a minimum brightness (e.g., 10) so it's not completely off in dark
  int brightness = map(lightVal, 0, 4095, 10, 255);
  ring.setBrightness(brightness);

  // Visual feedback based on current mood
  // Only update visual effect if it's different from the last one or if a full cycle is needed
  static int lastVisualMoodIndex = -1;
  static unsigned long lastVisualUpdate = 0;
  const long visualUpdateInterval = 100; // Update visual effect every 100ms for smoother animation

  if (currentMoodIndex != lastVisualMoodIndex || (currentMillis - lastVisualUpdate > visualUpdateInterval)) {
    switch (currentMoodIndex) {
      case 0: animateRainbow(); break;       // Happy
      case 1: animatePulse(currentMood.color); break; // Sad
      case 2: flashColor(currentMood.color, 1); break; // Angry (reduced flashes for smoother loop)
      case 3: animateWave(currentMood.color); break;  // Curious
      case 4: animateStrobe(currentMood.color); break; // Excited
      case 5: animateFade(currentMood.color); break;  // Sleepy
      case 6: showColor(currentMood.color); break;    // Calm (static color)
      case 7: flashColor(currentMood.color, 1); break; // Anxious (reduced flashes)
      case 8: animateWave(currentMood.color); break;  // Grateful
      case 9: animateFade(currentMood.color); break;  // Focused
      case 10: animatePulse(currentMood.color); break; // Relaxed
      case 11: animateStrobe(currentMood.color); break; // Energetic
      default: showColor(currentMood.color); break; // Fallback
    }
    lastVisualMoodIndex = currentMoodIndex;
    lastVisualUpdate = currentMillis;
  }


  // Audio feedback (play melody at controlled interval)
  if (currentMillis - lastMelodyPlay >= melodyPlayInterval) {
    playMelody(currentMood.melody, currentMood.melodyLength);
    lastMelodyPlay = currentMillis; // Update last melody play time
  }
}

void updateDisplay() {
  Mood currentMood = moods[currentMoodIndex];

  display.clearDisplay(); // Clear the display buffer

  // Mood name with icon
  display.setCursor(0, 0);
  display.setTextSize(1); // Small text for "Mood:"
  display.print("Mood: ");
  display.setTextSize(2); // Larger text for mood name
  display.print(currentMood.name);

  // Mood icon (adjust position based on text size)
  display.setCursor(display.getCursorX() + 5, 0); // Move cursor right after mood name
  display.setTextSize(2);
  switch (currentMoodIndex) {
    case 0: display.print(":)"); break;  // Happy
    case 1: display.print(":("); break;  // Sad
    case 2: display.print(">:("); break; // Angry (more expressive)
    case 3: display.print("?"); break;   // Curious
    case 4: display.print("!"); break;   // Excited (single exclamation for space)
    case 5: display.print("Zz"); break;  // Sleepy
    case 6: display.print("~"); break;   // Calm
    case 7: display.print("??"); break;  // Anxious
    case 8: display.print("<3"); break;  // Grateful
    case 9: display.print("^^"); break;  // Focused
    case 10: display.print("~~"); break; // Relaxed
    case 11: display.print(">>"); break; // Energetic (more dynamic)
    default: display.print(":-)"); break; // Default icon
  }

  // Sensor data
  display.setTextSize(1); // Back to small text for sensor data
  display.setCursor(0, 20);
  display.print("Temp: "); display.print(temperature, 1); display.println(" C");
  display.print("Light: "); display.println(lightVal);
  display.print("Motion: ");
  // Calculate motion magnitude on the fly for display
  display.print(sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ), 1);
  display.println(" m/s^2"); // Corrected unit

  // Mood quote (scroll if too long, or truncate gracefully)
  display.setCursor(0, 45);
  display.print("\"");
  // Calculate max characters that fit on one line (approx 21 chars for size 1 text)
  // Fixed: Replaced display.getTextSize() with the known text size 1.
  int maxChars = (SCREEN_WIDTH / (1 * 6)) - 2; // 6 pixels per char + 2 for quotes
  display.print(currentMood.quote.substring(0, min((int)currentMood.quote.length(), maxChars)));
  display.print("\"");

  display.display(); // Push buffer to OLED
}

void showIntroAnimation() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print("Emotional");
  display.setCursor(30, 40);
  display.print("Lamp");
  display.display();

  // Rainbow animation during intro
  for (int i = 0; i < 3; i++) {
    animateRainbow(); // Play rainbow animation multiple times
    delay(300);       // Short delay between animation cycles
  }

  ring.clear(); // Turn off all NeoPixels
  ring.show();
}

void showColor(uint32_t color) {
  ring.fill(color); // Fill all pixels with the specified color
  ring.show();      // Update NeoPixel ring
}

void flashColor(uint32_t color, int flashes) {
  for (int i = 0; i < flashes; i++) {
    showColor(color); // Turn on color
    delay(150);       // Stay on for 150ms
    showColor(0);     // Turn off (black)
    delay(150);       // Stay off for 150ms
  }
}

void animateFade(uint32_t color) {
  uint8_t r = (color >> 16) & 0xFF; // Extract red component
  uint8_t g = (color >> 8) & 0xFF;  // Extract green component
  uint8_t b = color & 0xFF;         // Extract blue component

  // Fade in
  for (int i = 0; i <= 255; i += 5) { // Iterate from 0 to 255 (brightness)
    ring.fill(ring.Color(r * i / 255, g * i / 255, b * i / 255)); // Scale RGB values
    ring.show();
    delay(20);
  }
  // Fade out
  for (int i = 255; i >= 0; i -= 5) {
    ring.fill(ring.Color(r * i / 255, g * i / 255, b * i / 255));
    ring.show();
    delay(20);
  }
}

void animateWave(uint32_t color) {
  // A simple wave animation where one pixel lights up and moves around
  for (int i = 0; i < NUM_LEDS; i++) {
    ring.clear(); // Clear all pixels
    ring.setPixelColor(i, color); // Set current pixel to color
    ring.show();
    delay(50);
  }
  // Optional: wave back
  for (int i = NUM_LEDS - 1; i >= 0; i--) {
    ring.clear();
    ring.setPixelColor(i, color);
    ring.show();
    delay(50);
  }
}

void animateStrobe(uint32_t color) {
  // Rapid on-off effect
  for (int i = 0; i < 5; i++) { // 5 strobe cycles
    showColor(color);
    delay(50);
    showColor(0);
    delay(50);
  }
}

void animatePulse(uint32_t color) {
  // Brightness pulses up and down
  for (int i = 10; i <= 255; i += 15) { // Pulse up
    ring.setBrightness(i);
    showColor(color);
    delay(30);
  }
  for (int i = 255; i >= 10; i -= 15) { // Pulse down
    ring.setBrightness(i);
    showColor(color);
    delay(30);
  }
}

void animateRainbow() {
  // Corrected ColorHSV usage: hue (0-65535), saturation (0-255), value (0-255)
  // We want a full rainbow cycle, so hue should go through the full 65536 range.
  // Saturation and Value are kept at max (255) for vibrant colors.
  for (int j = 0; j < 65536; j += 256) { // Iterate through hue values (increments of 256 for speed)
    for (int i = 0; i < NUM_LEDS; i++) {
      // Distribute hue across the LEDs
      ring.setPixelColor(i, ring.gamma32(ring.ColorHSV((i * 65536 / NUM_LEDS + j), 255, 255)));
    }
    ring.show();
    delay(20); // Small delay for animation speed
  }
}

void playMelody(const int tones[], int length) {
  for (int i = 0; i < length; i++) {
    if (tones[i] != 0) { // If tone is not 0 (rest)
      tone(BUZZER_PIN, tones[i], 200); // Play tone for 200ms
      delay(250); // Wait slightly longer than tone duration for a gap
    } else {
      noTone(BUZZER_PIN); // Ensure no tone is playing for rests
      delay(250); // Rest duration
    }
  }
  noTone(BUZZER_PIN); // Stop tone after melody finishes
}

void playClickSound() {
  tone(BUZZER_PIN, 1000, 50); // Play a short 1kHz tone for 50ms
  delay(100);                 // Wait a bit after the tone
  noTone(BUZZER_PIN);         // Stop the tone
}
