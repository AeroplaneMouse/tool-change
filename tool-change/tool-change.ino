/*
 * This code is written by The Great Mr. Fly
 */
 
// Input from Mach 4
#define pinStepMach   2
#define pinDirMach    3
#define pinBit_1      6
#define pinBit_2      7
#define pinBit_3      8
#define pinBit_4      9
#define pinMachWait  12
#define pinMachToolChange 11

// Output to Mach 4
#define pinArduWait       13
#define pinToolChangeErr  A0

// Spindle
#define pinStepSpindle    4
#define pinDirSpindle     5
#define pinSpindleHome    10
#define pinAirBlast       A4
#define ALIGNMENT_OFFSET  180
#define STEPS_REV         2000 
#define REVS_TOOL         2

// Input from sensors
#define pinToolOne      A1
#define pinTool         A2
#define pinToolInPos    A3

bool interruptsAttached = false;
int sequenceCounter = 0;
int pinMachWaitLastState = LOW;
bool sequenceExecuted = false;
bool toolChangeErr = false;
bool toolChangeStarted = false;
int dir = 0;

int currentTool = 0;
int selectedTool = 0;

void setup() {
  // Setting pinmode.
  pinMode(pinStepMach, INPUT);
  pinMode(pinDirMach, INPUT);
  
  pinMode(pinStepSpindle, OUTPUT);
  pinMode(pinDirSpindle, OUTPUT);

  pinMode(pinBit_1, INPUT);
  pinMode(pinBit_2, INPUT);
  pinMode(pinBit_3, INPUT);
  pinMode(pinBit_4, INPUT);

  pinMode(pinToolOne, INPUT);
  pinMode(pinTool, INPUT);
  pinMode(pinToolInPos, INPUT);

  pinMode(pinMachWait, INPUT);
  pinMode(pinMachToolChange, INPUT);
  pinMode(pinArduWait, OUTPUT);

  // Starting serial connection.
  Serial.begin(9600);
  Serial.println("   ******   Tool changer   ******");
  
  //digitalWrite(pinDirSpindle, HIGH);
  //dir = 1;
  
  //homeSpindle();
  //delay(1000);
  //spindleStep(180);
  //toggleInterrupts();
}

void loop() {
  delay(100);
//  Serial.print(digitalRead(pinToolOne));
//  Serial.print(" : ");
//  Serial.print(digitalRead(pinTool));
//  Serial.print(" : ");
//  Serial.println(digitalRead(pinToolInPos));
  
  //if (true) return;

  if (Serial.available()) {
    selectedTool = (int)Serial.read() - 48;
    if (selectedTool == 0) currentTool = 0;
    Serial.print("Selected tool is now: ");
    Serial.println(selectedTool);
  }
  
  if (digitalRead(pinMachToolChange) == LOW) {
    toolChangeErr = false;
    digitalWrite(pinToolChangeErr, LOW);
    toolChangeStarted = false;
    if (!interruptsAttached) {
      toggleInterrupts();
      digitalWrite(pinDirSpindle, dir);
    }
    int dirState = digitalRead(pinDirMach);
    if (dirState != dir) {
      digitalWrite(pinDirSpindle, dirState);
      dir = dirState;
    }
    return;
  }
  else {
    // Resetting variables
    if (!toolChangeStarted) {
      //selectedTool = 0;
      sequenceExecuted = false;
      sequenceCounter = 0;
      pinMachWaitLastState = LOW;
      toolChangeStarted = true;
      if (interruptsAttached) toggleInterrupts();
    }
  }
  
  if (toolChangeErr) return;

  if (!sequenceExecuted) {
    Serial.print("Sequence number: ");
    Serial.println(sequenceCounter);
    int i, tmpCurrent = 0;
    int dist[] = {0, 1, 2, 3, 4, 5, 4, 3, 2, 1};
    switch (sequenceCounter) {
      case 0:
        // Loading selected tool number.
        selectedTool = decodeToolBits();
        Serial.print("Selected tool: ");
        Serial.println(selectedTool);
        if (selectedTool == 0) {
          currentTool = 0;
          sequenceCounter = -1;
          break;
        }
        delay(100);
        homeSpindle();
        spindleStep(ALIGNMENT_OFFSET);
        break;
      case 1:
//        if (digitalRead(pinToolInPos) == HIGH) {
//          digitalWrite(pinToolChangeErr, HIGH);
//          toolChangeErr = true;
//          Serial.println("ERROR. Tool is not untached");
//          return;
//        }
        // Finding tool 1
        if (currentTool == 0) {
          digitalWrite(pinDirSpindle, HIGH);
          for (i = 0; i < 10; i++) {
            Serial.print("Finding tool 1: ");
            Serial.println(i);
            if (digitalRead(pinToolOne) == LOW) {
              Serial.println("Tool 1 has been found...");
              currentTool = 1;
              break;
            }
            spindleStep(REVS_TOOL * STEPS_REV);
          }
          if (currentTool == 0) {
            digitalWrite(pinToolChangeErr, HIGH);
            toolChangeErr = true;
            Serial.println("Error finding tool 1...");
            return;
          }
        }
  
        //Calculating shortest route
        tmpCurrent = currentTool;
        for (i = 0; i < 10; i++) {
          if (tmpCurrent == selectedTool) break;
          tmpCurrent++;
          if (tmpCurrent == 11) tmpCurrent = 1;
        }
        if (i > 5) digitalWrite(pinDirSpindle, LOW);
        else digitalWrite(pinDirSpindle, HIGH);
        
        // Moving to selected tool
        Serial.print("Moving to tool ");
        Serial.println(selectedTool);
        spindleStep(dist[i] * REVS_TOOL * STEPS_REV);

        if (selectedTool == 1 && digitalRead(pinToolOne) != LOW) {
          Serial.print("Current tool is supposed to be 1, but it isn't...");
          currentTool = 0;
          return;
        }
        else currentTool = selectedTool;
        
        digitalWrite(pinDirSpindle, dir);
        //digitalWrite(pinAirBlast, HIGH);
        Serial.println("Starting air blast...");
        break;
      case 2:
        //digitalWrite(pinAirBlast, LOW);
        Serial.println("Stopping air blast...");
        break;
      default:
        Serial.print("ERROR!!! Sequence number: ");
        Serial.println(sequenceCounter);
        break;
    }
    digitalWrite(pinArduWait, HIGH);
    sequenceExecuted = true;
  }
  
  int pinMachWaitState = digitalRead(pinMachWait);
  if (pinMachWaitState == HIGH && pinMachWaitLastState == LOW) {
    sequenceExecuted = false;
    sequenceCounter++;
    digitalWrite(pinArduWait, LOW);
  }
  
  pinMachWaitLastState = pinMachWaitState;
}

int decodeToolBits() {
  bool bitStates[] = {(bool)digitalRead(pinBit_1), (bool)digitalRead(pinBit_2), (bool)digitalRead(pinBit_3), (bool)digitalRead(pinBit_4)};
  int bitValues[] = {1, 2, 4, 8};
  int toolNum = 0;
  for (int i = 0; i < 4; i++) {
    toolNum += bitStates[i] * bitValues[i];
  }
  return toolNum;
  //return 2;
}

void spindleStep(int steps) {
  for (int i = 0; i < steps; i++) {
    //Serial.println("STEPPING");
    digitalWrite(pinStepSpindle, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinStepSpindle, LOW);
    delayMicroseconds(50);
  }
}

void homeSpindle() {
  digitalWrite(pinDirSpindle, LOW);
  for (int i = 0; i < STEPS_REV + STEPS_REV / 10; i += 2) {
    if (digitalRead(pinSpindleHome) == LOW && i > 100) return;
    spindleStep(2);
    delayMicroseconds(25);
  }
}

int getPinVal(int pin) {
  return digitalRead(pin);
}

void toggleInterrupts() {
  // Attaching or dettaching interrupts on the step and dir pin from Mach 4
  if (interruptsAttached) {
    Serial.println("Dettaching interrupts...");
    interruptsAttached = false;
    detachInterrupt(digitalPinToInterrupt(pinStepMach));
    detachInterrupt(digitalPinToInterrupt(pinDirMach));
  }
  else {
    Serial.println("Attaching interrupts...");
    interruptsAttached = true;
    attachInterrupt(digitalPinToInterrupt(pinStepMach), repeatStep, RISING);
    attachInterrupt(digitalPinToInterrupt(pinDirMach), repeatDir, CHANGE);
  }
}

void repeatStep() {
  digitalWrite(pinStepSpindle, HIGH);
  delayMicroseconds(1);
  digitalWrite(pinStepSpindle, LOW); 
}

void repeatDir() {
  dir = (int)digitalRead(pinDirMach);
  digitalWrite(pinDirSpindle, dir);
  Serial.print("DIR!!! ");
  Serial.println((String)dir);
}

