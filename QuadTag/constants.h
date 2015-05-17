// Threshold after which the PWM input will be considered on
const short PWM_THRESHOLD = PWM_LOW + (PWM_HIGH - PWM_LOW) / 2;

// Duration the buzzer/LED should be activated for
const short INDICATOR_DURATION = 100;

// Time to wait before buzzer/LED activation
const short INDICATOR_INTERVAL = 100;

// Time to wait between pulses
const short FIRE_INTERVAL = 100;

// This pulse sets the threshold for a transmission start bit
const int START_BIT = 3000;

// This pulse sets the threshold for a transmission end bit
const int END_BIT = 2000;

// This pulse sets the threshold for a transmission that represents a 1
const int ONE = 1000;

// This pulse sets the threshold for a transmission that represents a 0
const int ZERO = 400;

// The amount of time to wait between pulses
const int PULSE_INTERVAL = 300;
