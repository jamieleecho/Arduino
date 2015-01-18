/**
 * Tennis
 * 
 * A Mintuduino version of the Tennis game found on Radio Shack's Science Fair Microcomputer Trainer
 * (28-260). Very high level documentation follows.
 *
 * To use this software, setup the Mintduino on a RadioShack Electronics Learning Lab Kit (280055).
 * Connect the cathodes of all of the LEDs on the learning lab to ground. Connect the annodes of the
 * LEDs to some digital lines on the Mintduino. These lights will serve as the tennis ball. Connect
 * weak pull up resistors to switches S1 and S4.  Connect the same side of the switches to digital
 * lines on the Mintduino. Connect the other side of the switches to ground. Connect the cathode of
 * buzzer to ground and the anode to a Mintduino digital line.
 *
 * For programming select:
 * o Arduino Duemilanovae w/ ATmega328
 * o Correct serial port
 * o USBtineyISP
 * o Do not select Burn Bootloader
 *
 * This was a morning exercise. Obviously, this is quick and dirty code.= and the key handling is
 * especially kludgey.
 *
 * References:
 * http://www.polylith.com/~brendan/ClassicComputers/Tandy/uCptrTrainManual1.html
 * http://2.bp.blogspot.com/-QNlXE60dVzo/Udy5_ZrySBI/AAAAAAAAADc/Bjbl2CJCwUQ/s1600/mintduino_sch-1088x766.png
 * http://makezine.com/projects/build-a-mintronics-mintduino/
 * http://www.ftdichip.com/Support/Documents/DataSheets/Cables/DS_TTL-232R_CABLES.pdf
 * 
 */

/** Pins that represent the balls */
static const int LEDPins[] = { 13, 12, 11, 10, 9, 8, 7 };

/** Middle pin */
static const int LEDNet = 10;

/** Mid index of LEDPins */
static const int LEDNETIndex = 3;

/** Number of LEDs */
#define NumLEDPins (sizeof(LEDPins)/sizeof(LEDPins[0]))

/** Left paddle pin */
static const int LeftPaddle = 5;

/** Right paddle pin */
static const int RightPaddle = 6;

/** Used for sound */
static const int BuzzerPin = 2; 

/** Length of time to wait for buzzer */
static const int BuzzerDelay = 1;

/** Length of time to buzz when ball was dropped */
static const int FailBuzzerDelay = 100;

/** Length of time to wait after failure */
static const int FailDelay = 250;

/** Maximum score either player can get before the game ends */
static const int MaxScore = 7;


/** Possible ball speeds */
enum BallSpeed {
  BallSpeedSlow = 100,
  BallSpeedMedium = 50,
  BallSpeedFast = 25
};

/** Directions of ball */
enum BallDirection {
  BallDirectionLeft,
  BallDirectionRight
};

/** Game modes */
enum GameMode {
  GameModePlay,
  GameModeWaitForLeft,
  GameModeWaitForRight
};

/** Easy index into ball speed */
static const BallSpeed BallSpeeds[] = {
  BallSpeedSlow, BallSpeedMedium, BallSpeedFast
};

/** Score of left player */
static int leftScore = 0;

/** Score of right player */
static int rightScore = 0;

/** Position of ball. Index into LEDPins */
static int ballPosition = 0;

/** Whether or not the left paddle key must be released */
static bool mustReleaseLeft = false;

/** Whether or not the left paddle key must be released */
static bool mustReleaseRight = false;

/** Ball direction */
static BallDirection ballDirection = BallDirectionRight;

/** speed of the ball */
static BallSpeed ballSpeed = BallSpeedSlow;

/** Current game mode */
static GameMode gameMode = GameModeWaitForLeft;


/** Resets the global state to start a new game */
void startNewGame() {
  leftScore = rightScore = ballPosition = 0;
  ballSpeed = BallSpeedSlow;
  gameMode = GameModeWaitForLeft;
  mustReleaseLeft = mustReleaseRight = true;
  refreshScreen();
}


/** Refresh the screen completely */
static void refreshScreen() {
  for(int ii=0; ii<NumLEDPins; ii++) {
    const int led = LEDPins[ii];
    digitalWrite(led, ((led == LEDNet) || (ii == ballPosition)) ? HIGH : LOW);
  }
}

/** Clear the screen completely */
static void clearScreen() {
  for(int ii=0; ii<NumLEDPins; ii++) {
    const int led = LEDPins[ii];
    digitalWrite(led, LOW);
  }
}

/**
 * Buzz for time.
 * @param time time to buzz in ms
 */
static void buzz(int time) {
  digitalWrite(BuzzerPin, HIGH);
  delay(time);
  digitalWrite(BuzzerPin, LOW);
}


/** Process key presses */
static void checkKeys() {
  int leftStatus = digitalRead(LeftPaddle);
  int rightStatus = digitalRead(RightPaddle);

  if (leftStatus == HIGH) mustReleaseLeft = false;
  if (rightStatus == HIGH) mustReleaseRight = false;

  // Look to see if the ball was hit
  if ((ballPosition < LEDNETIndex) && (ballDirection == BallDirectionLeft)) {
    if (leftStatus == LOW) {
      if (!mustReleaseLeft) {
        ballDirection = BallDirectionRight;
        ballSpeed = BallSpeeds[(LEDNETIndex - 1 - ballPosition)];
        mustReleaseLeft = true;
      }
    }

    if (rightStatus == LOW) 
      mustReleaseRight = true;
  } else if ((ballPosition > LEDNETIndex) && (ballDirection == BallDirectionRight)) {
      if (rightStatus == LOW) {
      if (!mustReleaseRight) {
        ballDirection = BallDirectionLeft;
        ballSpeed = BallSpeeds[(ballPosition - LEDNETIndex - 1)];
        mustReleaseRight = true;
      }
    }
    
    if (leftStatus == LOW) 
      mustReleaseLeft = true;
  }
}


/** the setup routine runs once when you press reset: */
void setup() {                
  // Initialize outputs
  for(int ii=0; ii<NumLEDPins; ii++)
    pinMode(LEDPins[ii], OUTPUT);     

  // initialize the buzzer
  pinMode(BuzzerPin, OUTPUT);     

  // initialize paddles
  pinMode(LeftPaddle, INPUT);     
  pinMode(RightPaddle, INPUT);     

  // Display the screen
  refreshScreen();

  // Make sure we are ready for a new game
  startNewGame();
}


/**
 * Waits for paddle to be pressed (pulled LOW) and released (return to HIGH).
 */
static void waitForPaddlePressAndRelease(int pin) {
  while(digitalRead(pin) == LOW);
  while(digitalRead(pin) == HIGH);
}


/** Display the left and right scores in binary format */
static void showScores() {
  digitalWrite(LEDPins[0], (leftScore & 0x04) ? HIGH : LOW);
  digitalWrite(LEDPins[1], (leftScore & 0x02) ? HIGH : LOW);
  digitalWrite(LEDPins[2], (leftScore & 0x01) ? HIGH : LOW);
  digitalWrite(LEDPins[3], LOW);
  digitalWrite(LEDPins[4], (rightScore & 0x04) ? HIGH : LOW);
  digitalWrite(LEDPins[5], (rightScore & 0x02) ? HIGH : LOW);
  digitalWrite(LEDPins[6], (rightScore & 0x01) ? HIGH : LOW);
}


/** Run when the ball is missed */
static void failSequence() {
  refreshScreen();
  buzz(FailBuzzerDelay);
  clearScreen();
  delay(FailDelay);
  refreshScreen();
  ballSpeed = BallSpeedSlow;
  mustReleaseRight = true;
  mustReleaseLeft = true;
  
  if ((leftScore >= MaxScore) || (rightScore >= MaxScore)) {
    showScores();
    buzz(FailBuzzerDelay);
    delay(FailDelay);
    buzz(FailBuzzerDelay);
    delay(FailDelay);
    waitForPaddlePressAndRelease(LeftPaddle);
    startNewGame();
  }
}


/** the loop routine runs over and over again forever: */
void loop() {
  // Wait for button to be pressed
  if (gameMode == GameModeWaitForLeft) {
    if (digitalRead(LeftPaddle) == LOW) {
      if (mustReleaseLeft) return;
    } else {
      mustReleaseLeft = false;
      return;
    }
    gameMode = GameModePlay;
    digitalWrite(LEDPins[ballPosition], LOW);
  } else if (gameMode == GameModeWaitForRight) {
    if (digitalRead(RightPaddle) == LOW) {
      if (mustReleaseRight) return;
    } else {
      mustReleaseRight = false;
      return;
    }
    gameMode = GameModePlay;
    digitalWrite(LEDPins[ballPosition], LOW);
  }
  
  // Move the ball
  if (ballDirection == BallDirectionRight) {
    ballPosition = ballPosition + 1;
    if (ballPosition >= NumLEDPins) {
      ballDirection = BallDirectionLeft;
      ballPosition = NumLEDPins - 1;
      leftScore++;
      gameMode = GameModeWaitForRight;
      failSequence();
      return;
    }
  } else {
    ballPosition = ballPosition - 1;
    if (ballPosition < 0) {
      ballDirection = BallDirectionRight;
      ballPosition = 0;
      rightScore++;
      gameMode = GameModeWaitForLeft;
      failSequence();
      return;
    }
  }
  
  // Display the ball
  const int led = LEDPins[ballPosition];
  digitalWrite(led, (led == LEDNet) ? LOW : HIGH);
  buzz(BuzzerDelay);
  delay(ballSpeed - BuzzerDelay);
  checkKeys();
    
  digitalWrite(led, (led == LEDNet) ? HIGH : LOW);
  delay(ballSpeed);
  checkKeys();
}

