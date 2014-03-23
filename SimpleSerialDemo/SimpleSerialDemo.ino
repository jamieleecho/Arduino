/*
  Simple Serial Port/LED demo
  
  This software is for demonstration purposes only. Use at your own risk.
  
  This program implements a simple command line interpreter that allows the
  user to turn elements in a tri-color LED on and off. To use this program,
  you must connect a RadioShack Full Color LED (276-028) or equivalent to
  an Arduino Uno. Connect its Vcc (long pin) to the Arduino 5V source via
  a 2.2K resistor. Connect the red, green and blue LED elements to Arduino
  pins 11, 10 and 9 respecitively.
  
  Use the Arduino Serial Monitor to control the LEDs. Make sure that you
  select the Newline and 9600 baud options in the Serial Monitor.
  Try sending the following commands
  red on
  green on
  blue on
  red off
  green off
  blue off
  
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

/** Red power level on [0, 255] lower values are brighter*/
static const int redPowerLevel = 175;

/** Green power level on [0, 255] lower values are brighter*/
static const int greenPowerLevel = 100;

/** Blue power level on [0, 255] lower values are brighter*/
static const int bluePowerLevel = 0;

/** Tokens for our very simple language */
static const int TokenRed     = 0;
static const int TokenGreen   = 1;
static const int TokenBlue    = 2;
static const int TokenOn      = 3;
static const int TokenOff     = 4;
static const int TokenUnknown = 5;

/**
 * Returns a pointer to the given character in buffer or NULL
 * if it was not found. Note that buffer is treated as a memory
 * block and not a string.
 *
 * @param buffer[in] buffer containing possibly unterminated data
 * @param c[in] character to find in buffer
 * @param sz[in] size of buffer
 * @return pointer to first occurance of c in buffer or NULL.
 */
static const char *memnchr(const char *buffer, char c, int sz) {
  for(int ii=0; ii<sz; ii++) {
    if (buffer[ii] == c)
      return  buffer + ii;
  }
  return NULL;
}

/**
 * Returns a single token for the contents in buffer.
 * @param buffer[in] input buffer
 * @param sz[in] size of buffer
 * @return Token corresponding to contents of buffer
 */
static int getToken(const char *buffer, int sz) {
  if (strncmp(buffer, "red", sz) == 0)
    return TokenRed;
  else if (strncmp(buffer, "green", sz) == 0)
    return TokenGreen;
  else if (strncmp(buffer, "blue", sz) == 0)
    return TokenBlue;
  else if (strncmp(buffer, "on", sz) == 0)
    return TokenOn;
  else if (strncmp(buffer, "off", sz) == 0)
    return TokenOff;
  return TokenUnknown;
}

/**
 * Tokenizes the contents of buffer into tokens.
 * @param tokens[out] tokens written to this buffer
 * @param tokenBufferSize[in] size of tokens
 * @param buffer[in] buffer containing text to tokenize
 * @param sz[in] number of characters in buffer
 * @return number of tokens written to tokens
 */
static int myTokenize(int *tokens, int tokenBufferSize, const char *buffer, int sz) {
  int numTokens = 0;
  boolean foundTokenStart = false;
  const char *tokenStart = NULL;
  int ii;
  for(ii=0; ii<sz; ii++) {
    char c = buffer[ii];
    boolean foundSpace = isspace(c);
    if (foundTokenStart) {
      if (foundSpace) {
        tokens[numTokens++] = getToken(tokenStart, buffer + ii - tokenStart - 1);
        foundTokenStart = false;
        if (numTokens >= tokenBufferSize)
          return numTokens;
      }
    } else {
      if (!foundSpace) {
        tokenStart = buffer + ii;
        foundTokenStart = true;
      }
    }
  }
  
  if (foundTokenStart) {
    tokens[numTokens++] = getToken(tokenStart, buffer + ii - tokenStart - 1);
  }

  return numTokens;
}

/**
 * Parses and evaluates the following commands:
 * [red/green/blue] [on/off]\n
 *
 * For example, "red on\n" would turn the red LED on.
 * "blue off\n" would turn the blue LED off.
 *
 * @param buffer[in/out] executes the command in buffer. Shifts
 *        buffer contents over after command is executed.
 * @param bufferIndex[in/out] current buffer index which is updated
 *        after commands are executed. 
 */
static void parseAndEval(char *buffer, int &bufferIndex) {
  // Token buffer for parsing
  static int tokens[100];
  static const int tokenBufferSize = sizeof(tokens)/sizeof(tokens[1]);
  
  while(true) {
    // Extract the command line
    const char *endOfCommand = memnchr(buffer, '\n', bufferIndex);
    
    // No command?
    if (endOfCommand == NULL) {
      // Was the given command really long?
      if (bufferIndex >= 400) {
        bufferIndex = 0;
        Serial.write("Input line too long\n");
      }
      return;
    }    

    // Tokenize
    int sz = endOfCommand - buffer;
    int numTokens = myTokenize(tokens, tokenBufferSize, buffer, sz);
    if (numTokens <= 0)
      return; // nothing to do
    
    // Check syntax
    boolean isValidCommand = (numTokens == 2);
    isValidCommand = isValidCommand && ((tokens[0] == TokenRed) || (tokens[0] == TokenGreen) || (tokens[0] == TokenBlue));
    isValidCommand = isValidCommand && ((tokens[1] == TokenOn) || (tokens[1] == TokenOff));
    
    // Execute the command
    if (isValidCommand) {
      int led = (tokens[0] == TokenRed) ? redLED : ((tokens[0] == TokenGreen) ? greenLED : blueLED);
      boolean onOff = (tokens[1] == TokenOn);
      int val = onOff ? ((tokens[0] == TokenRed) ? redPowerLevel : ((tokens[0] == TokenGreen) ? greenPowerLevel : bluePowerLevel)) : 255;
      analogWrite(led, val);
      Serial.write("OK\n");
    } else {
      Serial.write("Syntax error\n");
     }
    
    // Skip to next command
    if (sz >= bufferIndex) {
      bufferIndex = 0;
      return;
    } else {
      memmove(buffer, endOfCommand + 1, bufferIndex - sz - 1);
      bufferIndex = bufferIndex - sz - 1;
    }
  }
}

/**
 * This function reads data from Serial, evaluates the command
 * and prints the result.
 */
static void readEvalPrint() {
  static char buffer[400];
  static int bufferIndex = 0;
  static int buffSize = sizeof(buffer);
  
  // Determine whether or not there are characters to read
  int bytesToRead = Serial.available();
  if (bytesToRead <= 0) return;
  
  // Make sure we don't read too much
  if (bytesToRead + bufferIndex >= buffSize)
    bytesToRead = buffSize - bufferIndex;

  // Read the data. readByte() hangs for some reason
  bufferIndex += Serial.readBytes(buffer + bufferIndex, bytesToRead);
    
  // Parse and evaluate the commands
  parseAndEval(buffer, bufferIndex);
}

/**
 * Initialize the hardware
 */
void setup() {
  Serial.begin(9600);
  pinMode(redLED, OUTPUT);  
  analogWrite(redLED, 255);  
  
  pinMode(greenLED, OUTPUT);     
  analogWrite(greenLED, 255);  
  
  pinMode(blueLED, OUTPUT);
  analogWrite(blueLED, 255);  
}

/**
 * Implements a simple read-eval-print loop.
 */
void loop() {
  readEvalPrint();
}
