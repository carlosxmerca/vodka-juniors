const int trigPin = D5; 
const int echoPin = D6; 

long duration;
int distance;

void setup() {
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200); // Starts the serial communication
}

/*
Cuchumbo: 14 cm
Hasta el sensor hay: 26 cm
Nivel: 9 cm

LLeno: 9 cm
Medio: 4 cm
Por vaciarse: 2 cm
Vacio: 0 cm
*/

int total_distance = 24;
int total_deposito = 14;
int level;

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculating the distance
  distance = duration/58;
  level = total_distance - distance;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print("Level: ");
  Serial.println(level);

  if (level > total_deposito){
    Serial.println("Error de distancia!!");
  } else if (level >= 9) {
    Serial.println("Deposito lleno!");
  } else if (level >= 4) {
    Serial.println("Deposito capacidad media!");
  } else if (level >= 2) {
    Serial.println("Deposito casi vacio!");
  } else {
    Serial.println("Deposito vacio!");
  }
  
  delay(2000);
}
