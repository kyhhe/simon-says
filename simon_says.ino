/* July 8 2025 - Simon Says game on Arduino UNO using 4 LEDs and a joystick input */

#include <stdio.h>

// define pins
#define VRx A0
#define VRy A1
#define SW 2
#define YELLOW 8
#define GREEN 9
#define RED 10
#define BLUE 11

// Define direction thresholds
#define LEFT_THRESHOLD  400
#define RIGHT_THRESHOLD 800
#define UP_THRESHOLD    400
#define DOWN_THRESHOLD  800

// Define commands
#define COMMAND_NO     0
#define COMMAND_LEFT   1
#define COMMAND_RIGHT  2
#define COMMAND_UP     3
#define COMMAND_DOWN   4

#define MAX_SIZE 100

// Declare variables
int X_CENTER;
int Y_CENTER;

int xVal = 0;
int yVal = 0;
int command = COMMAND_NO;
int sequence[MAX_SIZE];         // Simon's sequence
int playerSequence[MAX_SIZE];   // Player's sequence
int index = 0;
int lastInput = 0;

unsigned long startMillis;
const unsigned long debounceDelay = 50;
bool lastButtonState = LOW;
bool buttonState = LOW;
unsigned long lastDebounceTime = 0;

int gameRound = 0;      // Counts the current round of the game
bool gameOn = false;    // Keeps track whether the game has started or not

// Reads the current joystick input. Returns int of the corresponding direction (up down left right)
int joystickRead() {
  xVal = analogRead(VRx);
  yVal = analogRead(VRy);

  int dx = abs(xVal - X_CENTER);
  int dy = abs(yVal - Y_CENTER);

  if (dx > dy) {
    if (xVal > RIGHT_THRESHOLD) {
        return COMMAND_RIGHT;
      }
      else if (xVal < LEFT_THRESHOLD) {
        return COMMAND_LEFT;
      }
  }

  else {
    if (yVal > DOWN_THRESHOLD) {
        return COMMAND_DOWN;
      }
      else if (yVal < UP_THRESHOLD) {
        return COMMAND_UP;
      }
  }

  return COMMAND_NO;
}

// Debounced button read. Takes param of the button pin
bool debounceButton(int buttonPin) {
  // Read the button pin
  bool reading = digitalRead(buttonPin);
	
  // If the current reading is different from the last button state...
  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // reset debounce timer
  }

  // If enough time has passed since the debounce timer has been reset...
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // check if the reading is still different
    if (reading != buttonState) {
      // Update the button state to the reading
      buttonState = reading;
      
      // Button has gone from low -> high
      if (buttonState == LOW) {
        return true;  // valid press
      }
    }
  }
	
  // Update the last button reading
  lastButtonState = reading;
  return false;
}

// Turns off all LEDs
void ledOff() {
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW); 
}

// Turns on all LEDs
void ledOn() {
  digitalWrite(YELLOW, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, HIGH); 
}

// Calculates delay for playing LED sequence
// Param: yint - starting millis
//        x - x value to which the delay is calculated 
int calculateDelay(float yint, int x) {
  float decayRate = 0.9;
  return (yint * pow(decayRate, x));
}

// Play sequence of inputs
// Param: arr[] - sequence array
//        endIndex - last index of the sequence to play
void ledPlay(int arr[], int endIndex) {
  int i = 0;

  // Play each element in the sequence until the end index
  while (arr[i] != 0 && i <= endIndex){

    Serial.print(".");

    // Ensure all other LEDs are off
    ledOff();

    // Play the corresponding LED
    switch (arr[i]){
      case 1:
        digitalWrite(RED, HIGH);
        break;
      case 2:
        digitalWrite(YELLOW, HIGH);
        break;
      case 3:
        digitalWrite(BLUE, HIGH);
        break;
      case 4:
        digitalWrite(GREEN, HIGH);
        break;
    }
    delay(calculateDelay(1000, endIndex));    // Time LED is on
    ledOff();
    delay(calculateDelay(500, endIndex));     // Time LED is off
    i++;
  }

  // Sequence has finished playing
  ledOff();
}

// Player input
void playerTurn(int gameRound) {
  // Loops until the last entry of the sequence
  while (index < gameRound && gameOn) {

    int input = joystickRead();   // Read the current joystick state

    // Set the current input as the last input if the last and current input have the LED ON. Ensures the LED does not change unless the joystick has been released.
    if (lastInput != 0 && lastInput != input && input != 0) {
      input = lastInput;
    }

    // The joystick has an input
    if (input != 0  ) {  

      // Display corresponding LED
      switch (input){
        case 1:
          ledOff();
          digitalWrite(RED, HIGH);
          break;
        case 2:
          ledOff();
          digitalWrite(YELLOW, HIGH);
          break;
        case 3:
          ledOff();
          digitalWrite(BLUE, HIGH);
          break;
        case 4:
          ledOff();
          digitalWrite(GREEN, HIGH);
          break;    
      }
    }

    // The joystick has no input
    else {

      // Turn off LED
      ledOff();
      delay(50);


      // If the just swiched off
      if (lastInput != input) {
        
        // If the end of the sequence has not been saved, Save the input in the player sequence 
        if (index < gameRound) {
          playerSequence[index] = lastInput;
        }
        
        // Check if the player input matches the sequence
        if (playerSequence[index] != sequence[index]) {
         gameOn = false;
         break;
        }
        
        // Increment index
        index++;
      }
    }

    // Store the last input
    lastInput = input;
  }
}

// Reset variables
void gameBegin() {
  index = 0;
  gameRound = 1;
  lastInput = 0;
  gameOn = true;

  Serial.println("---------");
}

void setup() {
  Serial.begin(9600);         // Begin Serial connection
  randomSeed(analogRead(A5)); // New random seed

  pinMode(SW, INPUT_PULLUP);  // Initialize pins
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);

  X_CENTER = analogRead(VRx);  // Calibrate joystick center
  Y_CENTER = analogRead(VRy);

  Serial.println();
  Serial.println("Welcome to Simon Says! Press the button to begin");
}

void loop() {

  // Check for initial game start
  if (debounceButton(SW)) {
    gameBegin();
  }
  
  // The game has started
  while (gameOn == true && gameRound <= MAX_SIZE) {

    // Print game message
    Serial.print("Round ");
    Serial.print(gameRound);
    Serial.print(" begin!");

    // Generate the next input in the sequence 
    sequence[gameRound - 1] = random(1, 5);
    Serial.println(" Simon says:");
    ledPlay(sequence, (gameRound - 1));

    // Prompt the players turn
    Serial.println(" Your turn!");
    index = 0;
    playerTurn(gameRound);

    // Move to next round
    ledOff();
    delay(500);
    gameRound++;
  }

  // You've lost your streak (game has ended)
  if (gameOn == false && gameRound > 0) {

    // Flash all lights three times
    for (int i = 0; i < 3; i++) {
      ledOn();
      delay(500);
      ledOff();
      delay(100);
    }

    // Print losing message
    Serial.println("You lose!");
    Serial.print("Your score: ");
    Serial.println(gameRound - 2);

    Serial.println("Play again?");
    while (true){
      if (debounceButton(SW)) {
        gameBegin();
        break;
      }
    }
  }

}
