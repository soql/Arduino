
void setup() {
  Serial.begin(115200);
  while(!Serial){
    continue;
  }
}
double readADC(int pin){
  int rawValue=analogRead(pin);
  return rawValue;
}

void loop() {
 int analogValue1;
 int analogValue2;
  // put your main code here, to run repeatedly:
  analogValue1=readADC(12);
 // analogValue2=readADC(13);
  Serial.println(analogValue1);
  //Serial.println(analogValue2);
  delay(200);
}
