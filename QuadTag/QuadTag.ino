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
  pinMode(PIN_SENSOR, INPUT);
  pinMode(PIN_PWM, INPUT);

  // Startup sound
  playNote(NOTE_E0, 125);
  playNote(NOTE_A0, 125);
}

void loop()  {
  // Get the current time
  unsigned long current_time = millis();

  // Read data from sensor
  // int result[2];
  // senseIR(result);

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

void senseIR(int result[]) {
  result[0] = -1;
  result[1] = -1;

  int who[4];
  int what[4];
  int end;

  // Wait for a start bit
  if (pulseIn(PIN_SENSOR, LOW, 50) < START_BIT) {
    return;
  }

  // Read data
  who[0]   = pulseIn(PIN_SENSOR, LOW);
  who[1]   = pulseIn(PIN_SENSOR, LOW);
  who[2]   = pulseIn(PIN_SENSOR, LOW);
  who[3]   = pulseIn(PIN_SENSOR, LOW);
  what[0]  = pulseIn(PIN_SENSOR, LOW);
  what[1]  = pulseIn(PIN_SENSOR, LOW);
  what[2]  = pulseIn(PIN_SENSOR, LOW);
  what[3]  = pulseIn(PIN_SENSOR, LOW);
  end      = pulseIn(PIN_SENSOR, LOW);

  if (end <= END_BIT) {
    Serial.print("Bad end bit: ");
    Serial.println(end);
    return;
  }

  int player = decodePacket(who);

  Serial.print('Player: ');
  Serial.println(player);

  int action = decodePacket(what);

  Serial.print('Action: ');
  Serial.println(action);

  result[0] = action;
  result[1] = player;
}

int decodePacket(int packet[]) {
  int decoded[4];
  for(int i = 0; i <= 3; i++) {
    int bit = getBitFromPulse(packet[i]);

    if (bit == -1) {
      // Bad data
      Serial.print("Failed to decode packet: ");
      Serial.print(packet[0]);
      Serial.print(packet[1]);
      Serial.print(packet[2]);
      Serial.print(packet[3]);
      return -1;
    }

    decoded[i] = bit;
  }

  return convert(decoded);
}

/**
  Get the value corresponding to a given pulse

  @param <int> pulse_duration
    The length of the pulse in microseconds
*/
int getBitFromPulse(int pulse_duration) {
  if (pulse_duration > ONE) {
    return 1;
  }
  else if (pulse_duration > ZERO) {
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
  Convert an array of 4 0s and 1s to a number

  @param <int[]> bits
    An array of 0s and 1s
*/
int convert(int bits[]) {
  int result = 0;
  int seed = 1;
  for (int i = 3; i >= 0; i--) {
    if (bits[i] == 1) {
      result += seed;
    }
    seed = seed * 2;
  }
  return result;
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
