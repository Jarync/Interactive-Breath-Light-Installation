#include <Wire.h>
#include <avr/wdt.h> // Include watchdog timer library
#include <Adafruit_NeoPixel.h>

// Define WS2812B LED pins and number of LEDs
#define LED_PIN_1 6       // 1st WS2812B data pin connected to digital pin 6
#define LED_PIN_2 5       // 2nd WS2812B data pin connected to digital pin 5
#define LED_PIN_3 3       // 3rd WS2812B data pin connected to digital pin 3
#define NUM_LEDS 60       // Number of LEDs per strip

// Define Adafruit_NeoPixel objects for three strips
Adafruit_NeoPixel strip1(NUM_LEDS, LED_PIN_1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2(NUM_LEDS, LED_PIN_2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip3(NUM_LEDS, LED_PIN_3, NEO_GRB + NEO_KHZ800);

// Use an array to manage multiple strips
Adafruit_NeoPixel* strips[] = { &strip1, &strip2, &strip3 };
const int numStrips = sizeof(strips) / sizeof(strips[0]); // Number of strips

// Sliding Average Filter function to smooth sensor readings
int slidingAverageFilterTime(int rawValue) {

  static int numReadings = 4;  // Number of samples to read

  static int readings[4] = { '\0' };  // Array to store readings
  static int index = 0;                // Current index
  static int total = 0;                // Sum of readings

  total = total - readings[index];    // Subtract the oldest reading
  readings[index] = rawValue;         // Store the current reading
  total = total + readings[index];    // Add the current reading
  index = (index + 1) % numReadings;  // Advance index

  int smoothedValue = total / numReadings;  // Calculate average
  return smoothedValue;
}

// ----------------------------- Microphone Sensor -------------------------------
#define soundPin             A0        // Microphone sensor analog input pin
#define acquisitionTime     1000       // Adaptive acquisition time (ms)
unsigned long startMillis = 0;         // Time tracking
int signalMax = 0;                     // Max signal value storage
int signalMin = 4096;                  // Min signal value storage
int sample;                            // Raw sample value
long runCount[2] = {0, 0};             // Run counters
long triangleValue = 0;                // Accumulated value
#define runMax 3                       // Count threshold to start detection
#define runMin 10                      // Count threshold to stop detection
int airflowValue = 0;                  // Airflow intensity value
// ----------------------------------------------------------------------

// Variables for LED control
int beatAvg = 0;                       // Variable to control LED (mapped from mic value)
unsigned long interval = 10;
unsigned long printTime = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  wdt_enable(WDTO_1S); // Enable watchdog timer with 1 second overflow time

  // Initialize all LED strips
  for (int i = 0; i < numStrips; i++) {
    strips[i]->begin();
    strips[i]->show(); // Initialize all pixels to 'off'
  }

  // ----------------------------- Microphone Sensor Initialization -------------------------------
  // Adaptive calibration to get current min and max values from microphone
  startMillis = millis(); // Record start time
  while (millis() - startMillis < acquisitionTime) {
    sample = analogRead(soundPin); // Read sensor data
    if (sample > signalMax) {
      signalMax = sample;
    }
    else if (sample < signalMin) {
      signalMin = sample;
    }
  }
  Serial.println("Microphone calibration completed.");
  // ----------------------------------------------------------------------
}

void loop() {
  wdt_reset(); // Reset watchdog timer

  // Get airflow change
  getAirflowChange();

  // Map airflowValue to beatAvg for LED control
  beatAvg = airflowValue;

  if ((millis() - printTime) > interval) {
    // Output airflowValue data for Serial Plotter
    printTime = millis();
    Serial.println(airflowValue); // Output airflowValue
  }

  // Control LEDs to simulate breathing effect
  if (airflowValue == 0) {
    // No airflow detected, turn off all strips
    for (int i = 0; i < numStrips; i++) {
      strips[i]->clear();
      strips[i]->show();
    }
  } else {
    // Control LEDs based on airflowValue magnitude
    // Map airflowValue to brightness and blink frequency
    int maxBrightness = map(beatAvg, 0, 150, 50, 255);
    maxBrightness = constrain(maxBrightness, 50, 255);

    // Calculate total period for one cycle (ms)
    float period = map(beatAvg, 0, 150, 2000, 500);
    period = constrain(period, 500, 2000);

    // Use state machine to control LED fade in and fade out
    static unsigned long prevTime = 0;
    static int state = 0; // 0: Brightness increasing, 1: Brightness decreasing

    unsigned long currentTime = millis();

    if (state == 0) {
      // Brightness increasing phase
      int brightness = ((currentTime - prevTime) * maxBrightness) / (period / 2);
      brightness = constrain(brightness, 0, maxBrightness);

      // Set brightness and color for all strips
      for (int i = 0; i < numStrips; i++) {
        strips[i]->setBrightness(brightness);

        // Set LED color (e.g., Red)
        uint32_t color = strips[i]->Color(255, 0, 0);
        for (int j = 0; j < NUM_LEDS; j++) {
          strips[i]->setPixelColor(j, color);
        }
        strips[i]->show();
      }

      // Check if brightness increase is complete
      if (currentTime - prevTime >= (period / 2)) {
        prevTime = currentTime;
        state = 1; // Switch to decreasing state
      }
    } else if (state == 1) {
      // Brightness decreasing phase
      int brightness = maxBrightness - ((currentTime - prevTime) * maxBrightness) / (period / 2);
      brightness = constrain(brightness, 0, maxBrightness);

      // Set brightness and color for all strips
      for (int i = 0; i < numStrips; i++) {
        strips[i]->setBrightness(brightness);

        // Set LED color (e.g., Red)
        uint32_t color = strips[i]->Color(255, 0, 0);
        for (int j = 0; j < NUM_LEDS; j++) {
          strips[i]->setPixelColor(j, color);
        }
        strips[i]->show();
      }

      // Check if brightness decrease is complete
      if (currentTime - prevTime >= (period / 2)) {
        prevTime = currentTime;
        state = 0; // Return to increasing state
      }
    }
  }

  delay(2); // Small delay to prevent excessive CPU usage
}

// Get airflow magnitude after processing microphone sensor data
void getAirflowChange() {
  // Get raw value
  sample = slidingAverageFilterTime(analogRead(soundPin)); // Get raw data value

  // Process waveform
  if (sample > signalMax) {             // If raw value is greater than max calibration value
    sample -= signalMax;                // Subtract max value
  } else if (sample < signalMin) {      // If raw value is less than min calibration value
    sample = abs(signalMin - sample);   // Get absolute difference
  } else {                              // Otherwise
    sample = 0;                         // Set to 0
  }

  // Triangle integration logic: Accumulate current data if sound condition is met
  if (sample > 0) {                     // If data is greater than 0
    if (++runCount[0] > runMax) {       // Increment counter, if greater than threshold
      runCount[1] = 0;                  // Reset stop counter
      triangleValue += sample;          // Accumulate value
    }
  } else if (sample == 0) {             // If data equals 0
    if (++runCount[1] > runMin) {       // Increment stop counter, if greater than threshold
      runCount[0] = 0;                  // Reset run counter
      runCount[1] = runMin;             // Set stop counter to min
      triangleValue = 0;                // Reset accumulated value
    }
  }

  // Get amplitude change
  if (triangleValue != 0) {             // If not equal to 0
    airflowValue = triangleValue / runCount[0];   // Calculate average
    // Constrain airflowValue range to prevent it from being too large
    airflowValue = constrain(airflowValue, 10, 150);
  } else {                              // If equal to 0
    airflowValue = 0;                   // Set to 0
  }
}