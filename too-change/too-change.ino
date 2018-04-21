/*
 * This code was written by The Great Mr. Fly
 */

// Input from Mach 4
#define pinStepMach 2
#define pinDirMach  3

// Output to Mach 4


// Spindle
#define pinStepSpindle  4
#define pinDirSpindle   5
#define stepOnTime      1
void setup() {
  // Setting pinmode.
  pinMode(pinStepMach, INPUT);
  pinMode(pinDirMach, INPUT);
  pinMode(pinStepSpindle, OUTPUT);
  pinMode(pinDirSpindle, OUTPUT);

  // Starting serial connection.
  Serial.begin(9600);
  Serial.println("   ******   Tool changer   ******");
}

void loop() {
  
}

}
