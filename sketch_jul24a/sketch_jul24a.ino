void setup() {
 Serial.begin(115200);
 pinMode(0, INPUT);
 pinMode(0, OUTPUT);
}

void loop() {
  
  digitalWrite(13,LOW);
  Serial.println(digitalRead(13));
  delay(50);
  digitalWrite(13,HIGH);
delay(50);
}
