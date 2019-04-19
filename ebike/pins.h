#ifndef __OUR_PINS__

#define __OUR_PINS__

// Output Pin Definitions
#define HEAD_LIGHT_OUT 9
#define BRAKE_LIGHT_OUT 10
#define LEFT_TURN_LIGHT_OUT 11
#define RIGHT_TURN_LIGHT_OUT 12
#define LED_PIN_OUT 13
#define BT_ENABLE 42

// Input pin definitions
#define LEFT_BRAKE_IN 25
#define RIGHT_BRAKE_IN 26
#define LEFT_TURN_IN 46
#define RIGHT_TURN_IN 48
#define HIGH_BEAM_IN 45
#define LOW_BEAM_IN 47

// Constants
#define FULL_LIGHT_INTENSITY 255 // Desired PWM brightness for headlight
#define NIGHT_LIGHT_INTENSITY 128 // Desired night-light intensity
#define BLINK_INTERVAL_MS 1000 // blink interval in MS (should toggle every second)

#endif
