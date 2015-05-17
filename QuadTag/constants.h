// The RC input port and pin
#define RC_PORT PINE
#define RC_PIN (1 << PE6)

// The interrupt that the RC pin triggers
#define RC_INTERRUPT 4

// The IR receiver input port and pin
#define IR_PORT PIND
#define IR_PIN (1 << PD0)

// The interrupt that the IR pin triggers
#define SENSOR_INTERRUPT 0

// Indicator digital PIN (13 is the built-in indicator LED)
#define PIN_LED 13

// Hit indicator digital PIN
#define PIN_HIT_LED 8

// Laser digital pin
#define PIN_LASER 6

// Buzzer analog pin
#define PIN_BUZZER 5

// Time to wait between laser pulses
#define FIRE_INTERVAL 100
