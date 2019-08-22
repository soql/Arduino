/*
 circuitdigest.com
 Sample STM32 Blink Program for Blue Pill board 
 */

// the setup function runs once when you press reset or power the board
void setup() {
  Serial1.begin(9600);
  // initialize digital pin PC13 as an output.
  pinMode(PC13, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  Serial1.println("ELO");
  digitalWrite(PC13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);              // wait for a second
  digitalWrite(PC13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);              // wait for a second
}
