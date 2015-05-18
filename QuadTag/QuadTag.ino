#include "constants.h"
#include "protocol.h"
#include "actions.h"
#include "songs.h"
#include "configuration.h"
#include <TimerOne.h>
#include <QueueList.h>

// Time at which the hit LED should be turned off
volatile unsigned long hitLED_off = 0;

// Time at which the indicator LED should be turned off
volatile unsigned long indicatorLED_off = 0;

// Hold notes and durations to play
QueueList <unsigned int> noteQueue;
volatile unsigned long nextNoteTime = 0;

// Time when the RC pin went high
volatile unsigned long rcPulseStart = 0;

// Whether RC input is currently above the threshold
volatile bool rcOn = false;

// Time when the IR pin went high
volatile unsigned long irPulseStart = 0;

// Whether we've started reading a pulse
volatile boolean irPacketStarted = true;

// The partially read IR packet
volatile char irBuffer = 0;

// The last complete IR packet
volatile int irPacket = -1;

// The last time the laser was fired
unsigned long lastFireTime = 0;

void setup()  {
  // Setup pins
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_HIT_LED, OUTPUT);
  pinMode(PIN_LASER, OUTPUT);

  // Listen for changes on the interrupt associated with the RC pin
  attachInterrupt(RC_INTERRUPT, handleRCPinChangeInterrupt, CHANGE);

  // Listen for changes on the interrupt associated with the IR sensor pin
  attachInterrupt(SENSOR_INTERRUPT, handleIRPinChangeInterrupt, CHANGE);

  // Initialize timer1 with a 16ms period
  Timer1.initialize(16000);
  Timer1.attachInterrupt(timerCallback);

  // Startup sound
  playSong(song_Charge);
}

void loop()  {
  // Get the current time
  unsigned long currentTime = millis();

  // Disable interrupts so we can read variables set in ISRs
  noInterrupts();

  // Check if we have a packet
  if (irPacket != -1) {
    // Extract the player and data from the packet
    unsigned int player = irPacket >> 4 & 15;
    unsigned int data = irPacket & 15;

    // Clear the packet to indicate we've processed it already
    irPacket = -1;

    Serial.print("Hit by player ");
    Serial.print(player);
    Serial.print(" with a ");
    Serial.print(data);
    Serial.println("!");

    // Flash LED and buzz
    flashHitLED(128);
    playNote(NOTE_A2, 32, 0);
  }

  // Re-enable interrupts
  interrupts();

  // Check if we can fire
  if (
    // Check if the trigger is pressed
    rcOn &&

    // Check that we're not firing too fast
    currentTime - FIRE_INTERVAL >= lastFireTime
  ) {
    // Fire lasers
    fire(PLAYER_ID, ACTION_LASER);

    // Store the fire time so we can limit fire reate
    lastFireTime = currentTime;
  }
}

/**
  Handle pin change interrupts for the RC input
*/
void handleRCPinChangeInterrupt() {
  unsigned long currentTime = micros();

  // Check if the RC pin is high
  if (RC_PORT & RC_PIN) {
    // Mark the start of the pulse
    rcPulseStart = currentTime;
  }
  else if (rcPulseStart > 0) {
    // Calculate the pulse length and determine if the RC input is on
    rcOn = (currentTime - rcPulseStart) > RC_THRESHOLD;

    // Reset the pulse start time
    rcPulseStart = 0;
  }
}

/**
  Handle pin change interrupts for the IR pin
*/
void handleIRPinChangeInterrupt() {
  unsigned long currentTime = micros();

  // Check if the IR pin is low
  if (!(IR_PORT & IR_PIN)) {
    // Note the time the pulse started
    irPulseStart = currentTime;
  }
  else if (irPulseStart > 0) {
    // Calculate the length of the pulse
    unsigned int pulseLength = currentTime - irPulseStart;

    // Reset the pulse start time
    irPulseStart = 0;

    if (pulseLength >= START_BIT) {
      // Mark that we've begun to receive a packet
      irPacketStarted = true;
      irBuffer = 0;
    }
    else if (irPacketStarted) {
      // Only bother looking for the rest of the packet if we got the start
      if (pulseLength >= END_BIT) {
        // Store the read packet
        irPacket = irBuffer;

        // Mark that
        irPacketStarted = false;
      }
      else if (pulseLength >= ONE) {
        // Got a one
        irBuffer = irBuffer << 1 | 1; // Shift all bits to the left one, and set the LSB to 1
      }
      else if (pulseLength >= ZERO) {
        // Got a zero
        irBuffer = irBuffer << 1; // Shift all bits to the left one, and leave the LSB 0
      }
      else {
        // Bad pulse length, ignore the rest of the packet
        irPacketStarted = false;
      }
    }
  }
}

/**
  Handle timer interrupts
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

/**
  Fire the IR "laser"

  @param <unsigned int> player
    The player ID
  @param <unsigned int> data
    Additional data
*/
void fire(unsigned int player, unsigned int data) {
  Serial.print("Player ");
  Serial.print(player);
  Serial.print(" firing a ");
  Serial.print(data);
  Serial.println("!");

  // Encode data as 1s and 0s
  char encoded[8];
  for (int i = 3; i >= 0; i--) {
    encoded[i] = player >> i & 1;
  }
  for (int i = 3; i >= 0; i--) {
    encoded[i + 4] = data >> i & 1;
  }

  // Turn on indicator LED
  digitalWrite(PIN_LED, HIGH);

  // Disable interrupts while we run
  noInterrupts();

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

  // Add a space for good measure
  delayMicroseconds(PULSE_INTERVAL);

  // Re-enable interrupts
  interrupts();

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
    delayMicroseconds(12);
    digitalWrite(pin, LOW);
    delayMicroseconds(12);
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
void flashHitLED(unsigned int duration) {
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
void flashIndicatorLED(unsigned int duration) {
  if (indicatorLED_off == 0) {
    digitalWrite(PIN_LED, HIGH);
  }
  indicatorLED_off = millis() + duration;
}
