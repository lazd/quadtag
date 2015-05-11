// Threshold after which the PWM input will be considered on
const short PWM_THRESHOLD = PWM_LOW + (PWM_HIGH - PWM_LOW) / 2;

// Signal value for buzzer output
const short BUZZER_OUTPUT = 128;

// Duration the buzzer/LED should be activated for
const short TIME_INDICATE = 100;

// Time to wait before buzzer/LED activation
const short TIME_INDICATE_WAIT = 100;

// Duration of a laser pulse
const short TIME_FIRE = 75;

// Time to wait between pulses
const short TIME_FIRE_WAIT = 45;
