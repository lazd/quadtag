#include "configuration.h"
#include "constants.h"
#include "notes.h"

// Value as read from the PWM input
unsigned long lastFireTime = 0;

void setup()  {
  // Setup pins
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);
  pinMode(PIN_SENSOR, INPUT);
  pinMode(PIN_PWM, INPUT);

  // Startup sound
  playNote(NOTE_E0, 125);
  playNote(NOTE_A0, 125);
}

void loop()  {
  // Get the current time
  unsigned long currentTime = millis();

  // Read data from sensor
  // int result[2];
  // senseIR(PIN_SENSOR, result);

  // Read PWM input value
  int pwmValue = pulseIn(PIN_PWM, HIGH);

  // Check if we can fire
  if (
    // @todo don't fire if we're being hit

    // Check if the trigger is pressed
    pwmValue > PWM_THRESHOLD &&

    // Check that we're not firing too fast
    currentTime - FIRE_INTERVAL >= lastFireTime
  ) {
    fire(PLAYER_ID, 0);
    lastFireTime = currentTime;
  }
}

/**
  Get a packet from the IR at the specified pin

  @param <int> pin
    The pin to read from
  @param <int[]> result
    The array the result should be stored in.
     * result[0] = playerId
     * result[1] = action
*/
void senseIR(int pin, int result[]) {
  result[0] = -1;
  result[1] = -1;

  // Wait for a start bit
  if (pulseIn(pin, LOW, 50) < START_BIT) {
    return;
  }

  // Read data
  int playerId = getInt(pin);
  int action = getInt(pin);
  int end = pulseIn(pin, LOW);

  if (end <= END_BIT) {
    Serial.print("Bad end bit: ");
    Serial.println(end);
    return;
  }

  Serial.print('Player ID: ');
  Serial.println(playerId);

  Serial.print('Action: ');
  Serial.println(action);

  result[0] = playerId;
  result[1] = action;
}

/**
  Read a 4 bit integer from the IR sensor at the specified pin

  @param <int> pin
    The pin to read from

  @returns <int> 4 bit integer
*/
int getInt(int pin) {
  int result = 0;

  for (int i = 0; i < 4; i++) {
    // Read the pulse from the sensor
    int pulseDuration = pulseIn(pin, LOW);

    // Get the bit represented by the pulse
    int bit = getBitFromPulse(pulseDuration);

    // Check for bad data
    if (bit == -1) {
      result = -1;
      Serial.print("Got invalid pulse: ");
      Serial.println(pulseDuration);
    }

    // Add the bit to the number
    if (result != -1) {
      who += bit << i;
    }
  }

  return result;
}

/**
  Get the value corresponding to a given pulse

  @param <int> pulseDuration
    The length of the pulse in microseconds

  @returns <int> 0, 1, or -1 for bad fara
*/
int getBitFromPulse(int pulseDuration) {
  if (pulseDuration > ONE) {
    return 1;
  }
  else if (pulseDuration > ZERO) {
    return 0;
  }
  else {
    return -1;
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
