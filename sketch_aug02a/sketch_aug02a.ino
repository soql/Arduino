void setup() {
  // Tutaj umiesc kod, który wykona się raz przy starcie układu:
Serial.begin(9600);
}

void loop() {
  // Tutaj umiesc kod, który bedzie wykonywał się w nieskonczonej petli:
Serial.println("Witaj swiecie!");
delay(500);
}
