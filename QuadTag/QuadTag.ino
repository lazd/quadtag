#include "configuration.h"
#include "constants.h"
#include "notes.h"

// Value as read from the PWM input
unsigned long last_fire_time = 0;

void setup()  {
  // Setup pins
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);
  pinMode(PIN_PWM, INPUT);

  // Startup sound
  playNote(NOTE_E0, 125);
  playNote(NOTE_A0, 125);
}

void loop()  {
  // Get the current time
  unsigned long current_time = millis();

  // @todo check if we're being hit

  // Read PWM input value
  int pwm_value = pulseIn(PIN_PWM, HIGH);

  // Check if we can fire
  if (
    // @todo don't fire if we're being hit

    // Check if the trigger is pressed
    pwm_value > PWM_THRESHOLD &&

    // Check that we're not firing too fast
    current_time - FIRE_INTERVAL >= last_fire_time
  ) {
    fire(PLAYER_ID, 0);
    last_fire_time = current_time;
  }
}

/**
  Fire the IR "laser"

  @param <int> player
    The player ID
  @param <int> data
    Additional data
*/
void fire(int player, int data) {
  Serial.println("Firing!");

  // Turn on the buzzer
  analogWrite(PIN_BUZZER, BUZZER_OUTPUT);

  // Turn on indicator LED
  digitalWrite(PIN_LED, HIGH);

  // Encode data as 1s and 0s
  int encoded[8];
  for (int i = 0; i < 4; i++) {
    encoded[i] = player >> i & B1;
  }

  for (int i = 4; i < 8; i++) {
    encoded[i] = data >> i & B1;
  }

  // Start transmission
  oscillationWrite(PIN_LASER, START_BIT);

  // Send separation bit
  digitalWrite(PIN_LASER, HIGH);
  delayMicroseconds(PULSE_INTERVAL);

  // Send data
  for (int i = 7; i >= 0; i--) {
    oscillationWrite(PIN_LASER, encoded[i] == 0 ? ZERO : ONE);

    // Send separation bit
    digitalWrite(PIN_LASER, HIGH);
    delayMicroseconds(PULSE_INTERVAL);
  }

  // End transmission
  oscillationWrite(PIN_LASER, END_BIT);

  // Turn off indicator LED
  digitalWrite(PIN_LED, LOW);

  // Turn off the buzzer
  analogWrite(PIN_BUZZER, LOW);
}

/**
  Write the given data to the IR transmitter at the specified pin

  @param <int> pin
    The pin to write to
  @param <int> data
    The data to write
*/
void oscillationWrite(int pin, int data) {
  for(int i = 0; i <= data / 26; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(13);
    digitalWrite(pin, LOW);
    delayMicroseconds(13);
  }
}

/**
  Flash the indicator LED and beep

  @param <short> times
    The number of times to flash
*/
void indicate(short times) {
  for (short i = 0; i < times; i++) {
    analogWrite(PIN_BUZZER, BUZZER_OUTPUT);
    digitalWrite(PIN_LED, HIGH);
    delay(INDICATOR_DURATION);

    analogWrite(PIN_BUZZER, LOW);
    digitalWrite(PIN_LED, LOW);
    delay(INDICATOR_INTERVAL);
  }
}

/**
  Play the specified tone for the specified duration

  @param <int> tone
    The tone to play in Hz + NOTE_BASE
  @param <int> duration
    The time to play the tone for in milliseconds
*/
void playNote(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(PIN_BUZZER, HIGH);
    delayMicroseconds(tone);
    digitalWrite(PIN_BUZZER, LOW);
    delayMicroseconds(tone);
  }
  delay(NOTE_INTERVAL);
}
