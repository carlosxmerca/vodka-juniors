int bomba = D1;

void setup() {
  // put your setup code here, to run once:
  pinMode(bomba, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(bomba, LOW);
  delay(2000);
  digitalWrite(bomba, HIGH);
  delay(1000);
}