// HTTP Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

// Sensors Libraries
#include <DHT.h>

// Adafruit Libraries
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

const char* ssid = "CLARO_CD928A";
const char* password = "Bca2cca84F";

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
#define AIO_USERNAME "MiguelAcosta"
#define AIO_KEY "aio_vcbK23kDltGp6mVNo0kaXeyfLOID"

//Your Domain name with URL path or IP address with path
String serverName = "https://vodkaapi.fly.dev/";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 1*1000*60;

// Pines
const int bomba = D1;
const int fc_sensor = A0;
const int trigPin = D5;
const int echoPin = D6;
// DHT pin
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Global var
int day, hour;
int Days[7] = {2,2,2,2,2,2,2}; 
String response;
int total_distance = 24;
int total_deposito = 14;
int level;
// Structs
struct responseDH{
    int day;
    int hour;
};
typedef struct{
  float humidity;
  float temperature;
} dhtResponse;

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client2;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client2, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/
// Setup a feed called 'photocell' for publishing.
Adafruit_MQTT_Publish user = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/usuario");
Adafruit_MQTT_Publish humidityChart = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humiditychart");
Adafruit_MQTT_Publish humiditySoil = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humiditysoil");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish waterIndicator = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/waterindicator");

void setup() {
  // Comenzamos el sensor DHT
  pinMode(fc_sensor, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  dht.begin();
  Serial.begin(115200); 

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 15 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
  // Send info to Adafruit
  MQTT_connect();
  mqtt.processPackets(10000);
  if (!mqtt.ping())
  { // ping the server to keep the mqtt connection alive
    mqtt.disconnect();
  }
  dhtResponse data = dht11();

  humidity.publish(data.humidity);
  humiditySoil.publish(data.temperature);
  humidityChart.publish(fc28());

  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if(WiFi.status() == WL_CONNECTED){
      // WiFiClient client;
      std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

      //client->setFingerprint(fingerprint);
      // Or, if you happy to ignore the SSL certificate, then use the following line instead:
      client->setInsecure();
    
      HTTPClient http;
      String serverPath = serverName + "time";
      // String serverPath = serverName + "message/1";
      
      // Your Domain name with URL path or IP address with path
      http.begin(*client, serverPath.c_str());
      
      http.addHeader("Content-Type", "application/json");

      // If you need Node-RED/server authentication, insert user and password below
      // http.setAuthorization("AC35f55d787a8a836461c3c932ca9ce61d", "5b6fcd2abc02fa9ec8522f352f173c31");
      String body = ""; 
         
      // Send HTTP GET request
      int httpResponseCode = http.GET();
      // int httpResponseCode = http.POST(body);
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        // Serial.println(response);

        responseDH res = funRes(response);
        int hour = res.hour;
        int day = res.day;
        /*
        Serial.println(hour);
        Serial.println(day);
        */
        // Watering - routine
        wateringRoutine(day, hour);
        ultrasonico();
        delay(1000);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

responseDH funRes(String response){
    responseDH dh;
    int index = response.indexOf(',');
    String hourS = response.substring(0,index);
    String dayS = response.substring(index,response.length()-1);

    dh.hour = hourS.toInt();
    dh.day = dayS.toInt();
    return dh;
}

void wateringRoutine(int day, int hour){
  if((millis() - lastTime) > timerDelay){
    if((hour == 5 || hour == 18) && Days[day] != 0) {
      //watering();
      Serial.println("Regando...");
      Days[day]--;
    }
    // Setting the quantity of watering to 2 for each day
    if(Days[0] == 0 && hour == 2) {
      for(int i = 0; i < 7; i++)
        Days[i] = 2;
    }
    lastTime = millis();
  }
}

void watering(){
  // put your main code here, to run repeatedly:
  digitalWrite(bomba, LOW);
  delay(2000);
  digitalWrite(bomba, HIGH);
  delay(1000);
}

// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect()
{
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected())
  {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0)
  { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(10000); // wait 10 seconds
    retries--;
    if (retries == 0)
    { // basically die and wait for WDT to reset me
      while (1)
        ;
    }
  }
  Serial.println("MQTT Connected!");
}

dhtResponse dht11()
{
  // Esperamos 5 segundos entre medidas
  delay(5000);

  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();

  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t))
  {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return  {0, 0};
  }

  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humedad: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C \n");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C \n");

  return {h, t};
}

int fc28()
{
  int lectura = analogRead(fc_sensor);
  lectura = map(lectura, 1024, 200, 0, 100);
  /*
  AIRE: 1024
  TIERRA SECA: 919
  AGUA: 327
  */
  Serial.print("Humedad de la tierra: ");
  Serial.println(lectura);
  Serial.println();
  return lectura;
}

void ultrasonico(){
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  int duration = pulseIn(echoPin, HIGH);

  // Calculating the distance
  int distance = duration / 58;
  level = total_distance - distance;

  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print("Level: ");
  Serial.println(level);

  if (level > total_deposito){
    Serial.println("Error de distancia!!");
    waterIndicator.publish("Error de distancia!!");
  }else if (level >= 9){ 
    Serial.println("Deposito lleno!");
    waterIndicator.publish("Deposito lleno!");
  }else if (level >= 4){
    Serial.println("Deposito capacidad media!");
    waterIndicator.publish("Deposito capacidad media!");
  }else if (level >= 2){
    Serial.println("Deposito casi vacio!");
    waterIndicator.publish("Deposito casi vacio!");
  }else{
    Serial.println("Deposito vacio!");
    waterIndicator.publish("Deposito vacio!");
  }
}