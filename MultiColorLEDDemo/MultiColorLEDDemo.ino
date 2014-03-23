/*
  Multi-Color LED demo
  
  This software is for demonstration purposes only. Use at your own risk.
  
  This program demonstrates how to cycle through some of the different colors
  that a tri-color LED can generate. To use this program, you must connect
  a RadioShack Full Color LED (276-028) or equivalent to an Arduino Uno. Connect
  its Vcc (long pin) to the Arduino 5V source via a 2.2K resistor. Connect
  the red, green and blue LED elements to Arduino pins 11, 10 and 9
  respectively.
  
  We use LEDPowerTick() to cycle the LEDs. This has the advantage of potentially 
  saving the PWM channels, at the expense of using up more CPU power. Typically
  we would use PWM to cycle the LEDs which would free up the CPU for other things
  and make the transitions look smoother.
  
  Note that because we are using only 1 resistor, the amount of light generated
  by the red, green and blue elements is not balanced. We try to balance them
  here by adjusting the duty cycle for each one. Try adjusting redPowerLevel,
  greenPowerLevel and bluePowerLevel to improve the color balance.
 */

/** Red LED pin */
static const int redLED = 11;

/** Green LED pin */
static const int greenLED = 10;

/** Blue LED pin */
static const int blueLED = 9;

/** Red power level on [0, 15] higher values are brighter*/
static const int redPowerLevel = 5;

/** Green power level on [0, 15] higher values are brighter*/
static const int greenPowerLevel = 10;

/** Blue power level on [0, 15] higher values are brighter*/
static const int bluePowerLevel = 15;

/**
 * Turn on the specified led.
 * @param red[in] whether or not to turn on the red led
 * @param green[in] whether or not to turn on the green led
 * @param blue[in] whether or not to turn on the blue led
 */
static void turnOnLEDs(boolean red, boolean green, boolean blue) {
  digitalWrite(redLED, red ? LOW : HIGH);
  digitalWrite(greenLED, green ? LOW : HIGH);
  digitalWrite(blueLED, blue ? LOW : HIGH);  
}

/** power to the red LED [0->255]*/
int redLEDPower = 128;
/** power to the green LED [0->255]*/
int greenLEDPower = 128;
/** power to the blue LED [0->255]*/
int blueLEDPower = 128;

/**
 * Sets the desired power for each LED
 * @param red[in] red LED power [0->16]
 * @param green[in] green LED power [0->16]
 * @param blue[in] blue LED power [0->16]
 */
static void setLEDPower(int red, int green, int blue) {
  redLEDPower = red;
  greenLEDPower = green;
  blueLEDPower = blue;
}

/**
 * Invoke periodically to set the power to the LEDs
 */
static void LEDPowerTick() {
  static int ledPowerCounter = 0;
  int reducedCounter = ledPowerCounter & 15;
  turnOnLEDs(redLEDPower > reducedCounter, greenLEDPower > reducedCounter, blueLEDPower > reducedCounter);
  ledPowerCounter++;
}

/**
 * Initialize the hardware
 */
void setup() {                
  pinMode(redLED, OUTPUT);     
  pinMode(greenLED, OUTPUT);     
  pinMode(blueLED, OUTPUT);     
}

/**
 * Each loop iteration cycles through various LED colors.
 */
void loop() {
  // Power levels for Red, Green and Blue LEDs respectively.
  // Each array should contain the same number of elements
  static const int redLevels[]   = {  redPowerLevel, 0, 0, redPowerLevel, redPowerLevel, 0, redPowerLevel };
  static const int greenLevels[] = {  0, greenPowerLevel, 0, greenPowerLevel, 0, greenPowerLevel, greenPowerLevel };
  static const int blueLevels[]  = {  0, 0, bluePowerLevel, 0, bluePowerLevel, bluePowerLevel, bluePowerLevel };
  
  // Number of ticks to wait before starting a transition to the next level
  static const int ticksUntilTransitions = 2000;
  
  // Number of ticks to wait while transitioning to the next level
  static const float ticksBetweenTransitions = 2000;
  
  // Total number of ticks for each transition
  static const int ticksTotal = ticksUntilTransitions + (int)ticksBetweenTransitions;
  
  // Total number of levels (no need to change this)
  static const int numLevels = sizeof(redLevels)/sizeof(redLevels[0]);
  
  // Current index into redLevels, greenLevels and blueLevels
  static int currentLevelIndex = 0;
  
  // Index into the next redLevel, greenLevel and blueLevel
  static int nextLevelIndex = 1;
  
  // Current tick. Shuold always be on [0, ticksTotal]
  static int currentTick = 0;

  // If we have not reached the transition point, simply set the LED powers
  // specified by currentLevelIndex
  if (currentTick < ticksUntilTransitions) {
    setLEDPower(redLevels[currentLevelIndex], greenLevels[currentLevelIndex], blueLevels[currentLevelIndex]);
  } else {
    // Otherwise, perform a linear transition between currentLevelIndex and nextLevelIndex
    float percentTransition = (currentTick - ticksUntilTransitions)/ticksBetweenTransitions;
    float percentLeft = 1.0f - percentTransition;
    setLEDPower((int)((percentLeft * redLevels[currentLevelIndex]) + (percentTransition * redLevels[nextLevelIndex])),
                (int)((percentLeft * greenLevels[currentLevelIndex]) + (percentTransition * greenLevels[nextLevelIndex])),
                (int)((percentLeft * blueLevels[currentLevelIndex]) + (percentTransition * blueLevels[nextLevelIndex])));
  }

  // Increment the current tick
  currentTick++;
  
  // If we went over ticksTotal, begin to transition to the next level
  if (currentTick >= ticksTotal) {
    // Reset tick counter
    currentTick = 0;
    
    // Go to the next level
    currentLevelIndex = nextLevelIndex;
    
    // Remember to increment nextLevelIndex too and reset as needed
    nextLevelIndex++;
    if (nextLevelIndex >= numLevels)
      nextLevelIndex = 0;
  }
  
  // Power cycle the LEDs
  LEDPowerTick();
  
  // Wait around a little bit
  delay(1);
}
