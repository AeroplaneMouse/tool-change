/*
 * This code was written by The Great Mr. Fly
 */

// Input from Mach 4
#define pinStepMach 2
#define pinDirMach  3
#define pinBit_1    6
#define pinBit_2    7
#define pinBit_3    8
#define pinBit_4    9
#define pinWaiting  12

// Output to Mach 4
#define pinReady    13

// Spindle
#define pinStepSpindle  4
#define pinDirSpindle   5
#define stepOnTime      1

bool interruptsAttached = false;
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

  pinMode(pinWaiting, INPUT);
  pinMode(pinReady, OUTPUT);

  // Starting serial connection.
  Serial.begin(9600);
  Serial.println("   ******   Tool changer   ******");
  
  toggleInterrupts();
}

void loop() {
  
}

int decodeToolBits() {
  bool bitStates[] = {digitalRead(pinBit_1), digitalRead(pinBit_2), digitalRead(pinBit_3), digitalRead(pinBit_4)};
  int bitValues[] = {1, 2, 4, 8};
  int toolNum = 0;
  for (int i = 0; i < 4; i++) {
    toolNum += bitStates[i] * bitValues[i];
  }
  return toolNum;
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
  delayMicroseconds(stepOnTime);
  digitalWrite(pinStepSpindle, LOW); 
}

void repeatDir() {
  int state = (int)digitalRead(pinDirMach);
  digitalWrite(pinDirSpindle, state);
  Serial.print("DIR!!! ");
  Serial.println((String)state);
}

