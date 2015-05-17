#include "configuration.h"
#include "constants.h"
#include "songs.h"
#include <TimerOne.h>
#include <QueueList.h>

// Value as read from the PWM input
unsigned long lastFireTime = 0;

// Time at which the hitLED should be turned off
unsigned long hitLED_off = 0;
unsigned long indicatorLED_off = 0;

// Hold notes and durations to play
QueueList <unsigned int> noteQueue;
unsigned long nextNoteTime = 0;

void setup()  {
  // Setup pins
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_HIT_LED, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);
  pinMode(PIN_SENSOR, INPUT);
  pinMode(PIN_PWM, INPUT);

  // Startup sound
  playSong(song_Charge);

  // Initialize timer1 with a 16ms period
  Timer1.initialize(16000);
  Timer1.attachInterrupt(timerCallback);
}

void loop()  {
  // Get the current time
  unsigned long currentTime = millis();

  // Read data from sensor
  int result[2];
  senseIR(PIN_SENSOR, result);

  bool hit = false;
  if (result[0] != -1) {
    hit = true;

    Serial.print("Hit by player ");
    Serial.print(result[0]);
    Serial.println("!");

    // Flash LED and buzz
    flashHitLED(128);
    playNote(NOTE_A2, 32, 0);
  }

  // Read PWM input value
  int pwmValue = pulseIn(PIN_PWM, HIGH, 10000);

  // Check if we can fire
  if (
    // Don't fire if we're being hit
    !hit &&

    // Check if the trigger is pressed
    pwmValue > PWM_THRESHOLD &&

    // Check that we're not firing too fast
    currentTime - FIRE_INTERVAL >= lastFireTime
  ) {
    fire(PLAYER_ID, 3);
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
  int start = pulseIn(pin, LOW, 20000);
  if (start < START_BIT) {
    return;
  }

  // Turn on indicator LED
  digitalWrite(PIN_LED, HIGH);

  // Read data
  int playerId = getInt(pin);
  int action = getInt(pin);
  int end = pulseIn(pin, LOW);
  if (end <= END_BIT) {
    Serial.print("Bad end bit: ");
    Serial.println(end);

    // Turn off indicator LED
    digitalWrite(PIN_LED, LOW);
    return;
  }

  if (playerId == -1 || action == -1) {
    Serial.print("Got bad packet: ");

    Serial.print("[ ");
    Serial.print(playerId);
    Serial.print(", ");
    Serial.print(action);
    Serial.println(" ]");

    // Turn off indicator LED
    digitalWrite(PIN_LED, LOW);
    return;
  }
  else {
    Serial.print("Got packet: ");
    Serial.print("[ ");
    Serial.print(playerId);
    Serial.print(", ");
    Serial.print(action);
    Serial.println(" ]");
  }

  result[0] = playerId;
  result[1] = action;

  // Turn off indicator LED
  digitalWrite(PIN_LED, LOW);
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
      result += bit << i;
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

  @param <unsigned int> player
    The player ID
  @param <unsigned int> data
    Additional data
*/
void fire(unsigned int player, unsigned int data) {
  Serial.println("Firing!");

  // Turn on indicator LED
  digitalWrite(PIN_LED, HIGH);

  // Encode data as 1s and 0s
  int encoded[8];
  for (int i = 0; i < 4; i++) {
    encoded[i] = player >> i & B1;
  }

  for (int i = 0; i < 4; i++) {
    encoded[i + 4] = data >> i & B1;
  }

  // Start transmission
  oscillationWrite(PIN_LASER, START_BIT);

  // Send separation bit
  digitalWrite(PIN_LASER, HIGH);
  delayMicroseconds(PULSE_INTERVAL);

  // Send data
  for (int i = 0; i < 8; i++) {
    oscillationWrite(PIN_LASER, encoded[i] == 0 ? ZERO : ONE);

    // Send separation bit
    digitalWrite(PIN_LASER, HIGH);
    delayMicroseconds(PULSE_INTERVAL);
  }

  // End transmission
  oscillationWrite(PIN_LASER, END_BIT);

  // Turn off indicator LED
  digitalWrite(PIN_LED, LOW);
}

/**
  Write the given data to the IR transmitter at the specified pin

  @param <unsigned int> pin
    The pin to write to
  @param <int> data
    The data to write
*/
void oscillationWrite(unsigned int pin, int data) {
  for(int i = 0; i <= data / 26; i++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(13);
    digitalWrite(pin, LOW);
    delayMicroseconds(13);
  }
}

/**
  Play the specified tone for the specified duration

  @param <unsigned int> tone
    The tone to play in Hz
  @param <unsigned int> duration
    The time to play the tone for in milliseconds
  @param <unsigned int> duration
    The time to rest before the next note in milliseconds
*/
void playNote(unsigned int tone, unsigned int duration, unsigned int rest) {
  noteQueue.push(tone);
  noteQueue.push(duration);
  noteQueue.push(rest);
}

/**
  Play the specified tone for the specified duration

  @param <unsigned int[][3]> song
    The song to play
  @param <unsigned int[][0]> tone
    The tone to play in Hz
  @param <unsigned int[][1]> duration
    The time to play the tone for in milliseconds
  @param <unsigned int[][2]> duration
    The time to rest before the next note in milliseconds
*/
void playSong(unsigned int song[][3]) {
  for (int i = 0; i < sizeof(*song); i++) {
    unsigned int *note = song[i];
    playNote(note[0], note[1], note[2]);
  }
}

/**
  Turn the hit LED on for the specific duration

  @param <unsigned int> duration
    The time to turn the hit LED on for in milliseconds
*/
void flashHitLED(int duration) {
  if (hitLED_off == 0) {
    digitalWrite(PIN_HIT_LED, HIGH);
  }
  hitLED_off = millis() + duration;
}

/**
  Turn the indicator LED on for the specific duration

  @param <unsigned int> duration
    The time to turn the indicator LED on for in milliseconds
*/
void flashIndicatorLED(int duration) {
  if (indicatorLED_off == 0) {
    digitalWrite(PIN_LED, HIGH);
  }
  indicatorLED_off = millis() + duration;
}

/**
  Timer interrupt callback
*/
void timerCallback() {
  unsigned long currentTime = millis();
  if (hitLED_off != 0 && hitLED_off <= currentTime) {
    digitalWrite(PIN_HIT_LED, LOW);
    hitLED_off = 0;
  }

  if (indicatorLED_off != 0 && indicatorLED_off <= currentTime) {
    digitalWrite(PIN_LED, LOW);
    indicatorLED_off = 0;
  }

  if (!noteQueue.isEmpty() && nextNoteTime <= currentTime) {
    unsigned int note = noteQueue.pop();
    unsigned int duration = noteQueue.pop();
    unsigned int rest = noteQueue.pop();
    tone(PIN_BUZZER, note, duration);
    nextNoteTime = currentTime + duration + rest;
  }
}
