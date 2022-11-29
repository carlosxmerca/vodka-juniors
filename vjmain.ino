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
// Timer set to 10 mins -> test pourposes
unsigned long timerDelay = 10*1000*60;

// Horas de riego
int target1 = 20;
int target2 = 21;

// Pines
const int bomba = D8;
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
// Optimization
String payload;
String serverPath;
int notification;
bool waterState;
int httpResponseCode;
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

// Subscribe callback
Adafruit_MQTT_Subscribe waterOnOff = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/pumptoggle", MQTT_QOS_1);

void setup() {
  // Comenzamos el sensor DHT
  pinMode(fc_sensor, INPUT);
  pinMode(bomba, OUTPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  
  dht.begin();
  Serial.begin(115200); 
  digitalWrite(bomba, LOW);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 30 seconds, it will take 5 seconds before publishing the first reading.");

  waterOnOff.setCallback(onoffcallback);
  mqtt.subscribe(&waterOnOff);
}

void sendwamessage() {
  // WiFiClient client;
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  //client->setFingerprint(fingerprint);
  // Or, if you happy to ignore the SSL certificate, then use the following line instead:
  client->setInsecure();

  HTTPClient httpwa;

  int notificationwa = ultrasonico();
  String serverPathwa = serverName + "message/" + String(notificationwa);
  // Start client
  httpwa.begin(*client, serverPathwa.c_str());
  // Set content type
  httpwa.addHeader("Content-Type", "application/json");
  
  // Send HTTP POST request
  int httpResponseCodewa = httpwa.POST("");
  
  if (httpResponseCodewa>0) {
    Serial.print("HTTP Response code Wha message: ");
    Serial.println(httpResponseCodewa);
    Serial.println();
    payload = httpwa.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCodewa);
  }

  // Free resources
  httpwa.end();
}

void onoffcallback(char *data, uint16_t len)
{
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);
  String message = String(data);
  message.trim();
  if (message == "ON"){
    watering();
    sendwamessage();
  }
  if (message == "OFF"){
    digitalWrite(bomba, LOW);
  }
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
      serverPath = serverName + "time";
      
      // Start client
      http.begin(*client, serverPath.c_str());
      // Set content type
      http.addHeader("Content-Type", "application/json");
         
      // Send HTTP GET request
      httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        response = http.getString();
        // Serial.println(response);

        responseDH res = funRes(response);
        hour = res.hour;
        day = res.day;
        /*
        Serial.println(hour);
        Serial.println(day);
        */
        // Watering - routine
        waterState = wateringRoutine(day, hour);
        if (waterState){
          // sendwamessage(); -> send whatsapp message
          
          notification = ultrasonico();
          serverPath = serverName + "message/" + String(notification);
          // Start client
          http.begin(*client, serverPath.c_str());
          // Set content type
          http.addHeader("Content-Type", "application/json");
          
          // Send HTTP POST request
          httpResponseCode = http.POST("");
          
          if (httpResponseCode>0) {
            Serial.print("HTTP Response code Wha message: ");
            Serial.println(httpResponseCode);
            Serial.println();
            payload = http.getString();
          }
          else {
            Serial.print("Error code get time: ");
            Serial.println(httpResponseCode);
          }
          
        }
      }
      else {
        Serial.print("Error code time: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    // Update last time
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

// Watering routine -> water for 10 seconds
void watering(){
  digitalWrite(bomba, HIGH);
  delay(10000);
  digitalWrite(bomba, LOW);
  delay(200);
}

bool wateringRoutine(int day, int hour){
  if((millis() - lastTime) > timerDelay){
    if((hour == target1 || hour == target2) && Days[day] != 0) {
      Serial.println("Regando...");
      watering();
      Days[day]--;
      return true;
    }
    // Setting the quantity of watering to 2 for each day
    if(Days[0] == 0 && hour == 2) {
      for(int i = 0; i < 7; i++)
        Days[i] = 2;
    }
    // Update last time
    lastTime = millis();
  }
  return false;
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
  if (isnan(h) || isnan(t)){
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

int ultrasonico(){
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
    return 5;
  }else if (level >= 9){ 
    Serial.println("Deposito lleno!");
    waterIndicator.publish("Deposito lleno!");
    return 4;
  }else if (level >= 4){
    Serial.println("Deposito capacidad media!");
    waterIndicator.publish("Deposito capacidad media!");
    return 3;
  }else if (level >= 2){
    Serial.println("Deposito casi vacio!");
    waterIndicator.publish("Deposito casi vacio!");
    return 2;
  }else{
    Serial.println("Deposito vacio!");
    waterIndicator.publish("Deposito vacio!");
    return 1;
  }
}
