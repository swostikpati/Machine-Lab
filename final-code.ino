#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <Adafruit_NeoPixel.h>
#include <SD.h>

// These are the pins used for the music maker shield
#define SHIELD_RESET  -1      // VS1053 reset pin (unused!)
#define SHIELD_CS     7       // VS1053 chip select pin (output)
#define SHIELD_DCS    6       // VS1053 Data/command select pin (output)
#define CARDCS 4              // Card chip select pin
#define DREQ 3                // VS1053 Data request, ideally an Interrupt pin

// Motor and button pins
#define IN1 9
#define BUTTON_PIN 2

// neopixels
#define LED_PIN 12
#define LED_COUNT 64
#define MOTOR_SPEED 50


bool motorState = true;
bool musicPlaying = false;

Adafruit_VS1053_FilePlayer musicPlayer = 
    Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DCS, DREQ, CARDCS);


//neopixel setup
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
  public:
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type)
      : Adafruit_NeoPixel(pixels, pin, type)
    {
    }

    // Update the pattern
    void Update()
    {
      // No need for updates in this example
    }

    // Fade in and out from no color to color1 and back
    void Fade(uint32_t color1, uint16_t steps, uint8_t interval)
    {
      uint8_t startR = 0;
      uint8_t startG = 0;
      uint8_t startB = 0;

      // Fade in from no color to color1
      for (int i = 0; i < steps; i++)
      {
        uint8_t r = map(i, 0, steps - 1, startR, (color1 >> 16) & 0xFF);
        uint8_t g = map(i, 0, steps - 1, startG, (color1 >> 8) & 0xFF);
        uint8_t b = map(i, 0, steps - 1, startB, color1 & 0xFF);

        ColorSet(Color(r, g, b));
        show();
        delay(interval);
      }

      // Fade out from color1 to no color
      for (int i = steps - 1; i >= 0; i--)
      {
        uint8_t r = map(i, 0, steps - 1, startR, (color1 >> 16) & 0xFF);
        uint8_t g = map(i, 0, steps - 1, startG, (color1 >> 8) & 0xFF);
        uint8_t b = map(i, 0, steps - 1, startB, color1 & 0xFF);

        ColorSet(Color(r, g, b));
        show();
        delay(interval);
      }
    }

    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
      for (unsigned i = 0; i < numPixels(); i++)
      {
        setPixelColor(i, color);
      }
      show();
    }
};

NeoPatterns strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit VS1053 Simple Test");

  if (!musicPlayer.begin()) {
    Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
    while (1);
  }
  Serial.println(F("VS1053 found"));

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);
  }

  musicPlayer.setVolume(10, 10);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);

  pinMode(IN1, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  analogWrite(IN1, 0);
  strip.begin();
}

void loop() {
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(BUTTON_PIN);
  static unsigned long motorStartTime = 0;  // Stores when the motor was last toggled
  unsigned long currentTime = millis();
  // Define the desired color (e.g., bright blue)
  // uint32_t brightBlue = strip.Color(255, 0, 255);

  // // Kick off the pattern with a fade in and out
  // strip.Fade(brightBlue, 100, 15); // Adjust the steps (100) and interval (30) for the fade speed

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    delay(50); // Debounce delay
    motorState = !motorState;
    if (motorState) {
      analogWrite(IN1, 0);
      
    } else {
      analogWrite(IN1, MOTOR_SPEED);
      motorStartTime = currentTime;  // Reset the timer whenever the motor is started
    }

    if (!musicPlaying) {
      musicPlayer.startPlayingFile("/track001.mp3");
      musicPlaying = true;
    } else {
      musicPlayer.stopPlaying();
      musicPlaying = false;
    }
  }

  if (!motorState && currentTime - motorStartTime >= 45000) {
    motorState = !motorState;
    analogWrite(IN1, 0);  // Stop the motor
    musicPlaying = false;
    musicPlayer.stopPlaying();
  }

  lastButtonState = currentButtonState;
  delay(100);
}
