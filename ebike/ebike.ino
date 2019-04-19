#include <Arduino.h>
#include <avr/interrupt.h>

#include "interrupt.h"
#include "pins.h"

volatile byte headlightStatus = 0;
volatile byte brakeStatus = 0;
volatile byte lturnState = 0;
volatile byte rturnState = 0;
volatile byte hazardsState = 0;
volatile char hazardsStatus = 0; // 0 means off, 1 means on, -1 means just turned off
volatile unsigned long lturnPreviousMillis = 0;
volatile unsigned long rturnPreviousMillis = 0;
volatile unsigned long hazardsPreviousMillis = 0;

/*
   Setup functionality to create tasks, initialize timers, and setup outputs as needed
*/
void setup() {
  Serial.begin(9600);
  Serial1.begin(38400);
  digitalWrite(BT_ENABLE, HIGH);

  noInterrupts();

  // Initialize timers
  initialize_timer3_interrupt();
  initialize_timer4_interrupt();
  initialize_timer5_interrupt();

  // Create tasks
  createTask(brakeLightTask, HIGH_PRIORITY);
  createTask(hazardsTask, HIGH_PRIORITY);
  createTask(leftTurnTask, MEDIUM_PRIORITY);
  createTask(rightTurnTask, MEDIUM_PRIORITY);
  createTask(headLightTask, LOW_PRIORITY);
  interrupts();
}

/*
   Background loop which reads commands sent over bluetooth from an
   Android application.
*/
void loop() {

  // Bluetooth task
  if (Serial1.available()) {
    switch (Serial1.read()) {
      case '0':
        if (hazardsStatus == 1) {
          hazardsStatus = -1;
        }
        break;
      case '1':
        hazardsStatus = 1;
        break;
    }
  }
}

/**
   High priority task which is toggled using bluetooth button on an android phone.
   Toggles the blinkers on and off at a fixed rate.
*/
void hazardsTask() {
  if (hazardsStatus == 1) {
    unsigned long hazardsCurrentMillis = millis();
    if (hazardsCurrentMillis - hazardsPreviousMillis >= BLINK_INTERVAL_MS) {
      hazardsPreviousMillis = hazardsCurrentMillis;
      hazardsState = !hazardsState; // Toggle state
    }
    digitalWrite(RIGHT_TURN_LIGHT_OUT, hazardsState);
    digitalWrite(LEFT_TURN_LIGHT_OUT, hazardsState);

  } else if (hazardsStatus == -1) {
    hazardsState = LOW;
    hazardsStatus = 0;
    digitalWrite(RIGHT_TURN_LIGHT_OUT, hazardsState);
    digitalWrite(LEFT_TURN_LIGHT_OUT, hazardsState);
  }
}

/*
   Low priority task which handles logic for turning on and off
   the headlight
*/
void headLightTask() {

  // Read headlight pin status
  if (digitalRead(HIGH_BEAM_IN) == HIGH) {

    // If headlight was previously off, turn on
    if (headlightStatus == 0) {
      analogWrite(HEAD_LIGHT_OUT, FULL_LIGHT_INTENSITY);
      headlightStatus = 1;
    }
  } else if (headlightStatus == 1) { // if headlight was previously on, turn off
    analogWrite(HEAD_LIGHT_OUT, 0);
    headlightStatus = 0;
  }
}

/*
   High priority task which handles logic for turning on break lights
   and night lights when the headlight is on

*/
void brakeLightTask() {

  // Read in the brake signals
  byte brake = digitalRead(LEFT_BRAKE_IN) | digitalRead(RIGHT_BRAKE_IN);

  if (brake) {
    if (brakeStatus == 0) { // brakes not previously engaged, turn on now
      brakeStatus = 1;
      analogWrite(BRAKE_LIGHT_OUT, FULL_LIGHT_INTENSITY);
    }
  } else {
    if (brakeStatus == 1) { // brakes previously engaged, turn off now
      brakeStatus = 0;
    }

    // Running light if head light enabled, else off
    if (digitalRead(HIGH_BEAM_IN) == HIGH) {
      analogWrite(BRAKE_LIGHT_OUT, NIGHT_LIGHT_INTENSITY);
    } else {
      analogWrite(BRAKE_LIGHT_OUT, 0);
    }
  }
}

/*
   Medium priority task which handles logic for right turn signal
   does not trigger when hazards are on
*/
void rightTurnTask() {
  if (digitalRead(RIGHT_TURN_IN) == HIGH && hazardsStatus == 0) {
    unsigned long rturnCurrentMillis = millis();
    if (rturnCurrentMillis - rturnPreviousMillis >= BLINK_INTERVAL_MS) {
      rturnPreviousMillis = rturnCurrentMillis;
      rturnState = !rturnState; // Toggle state
      digitalWrite(RIGHT_TURN_LIGHT_OUT, rturnState);
    }
  } else if (hazardsStatus == 0) {
    rturnState = LOW; // Make sure lights turn off if turn signal not active
    digitalWrite(RIGHT_TURN_LIGHT_OUT, rturnState);
  }
}

/*
   Medium priority task which handles logic for left turn signal
   does not trigger when hazards are on
*/
void leftTurnTask() {
  if (digitalRead(LEFT_TURN_IN) == HIGH && hazardsStatus == 0) {
    unsigned long lturnCurrentMillis = millis();
    if (lturnCurrentMillis - lturnPreviousMillis >= BLINK_INTERVAL_MS) {
      lturnPreviousMillis = lturnCurrentMillis;
      lturnState = !lturnState; // Toggle state
      digitalWrite(LEFT_TURN_LIGHT_OUT, lturnState);
    }
  } else if (hazardsStatus == 0) {
    lturnState = LOW; // Make sure lights turn off if turn signal not active
    digitalWrite(LEFT_TURN_LIGHT_OUT, lturnState);
  }
}
