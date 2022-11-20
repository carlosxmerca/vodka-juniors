#include <ESP8266WiFi.h>
#include <DHT.h>

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN D2
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);
// Sensor ultrasonico
const int trigPin = D5;
const int echoPin = D6;

// Inicializacion de sensor fc28
const int fc_sensor = A0;

typedef struct{
  float humidity;
  float temperature;
} dhtResponse;

/************************* WiFi Access Point *********************************/

#define WLAN_SSID "S9"
#define WLAN_PASS "daniamigo"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883 // use 8883 for SSL
#define AIO_USERNAME "MiguelAcosta"
#define AIO_KEY "aio_vcbK23kDltGp6mVNo0kaXeyfLOID"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
// WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/
// Setup a feed called 'photocell' for publishing.
Adafruit_MQTT_Publish user = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/usuario");
Adafruit_MQTT_Publish humidityChart = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humiditychart");
Adafruit_MQTT_Publish humiditySoil = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humiditysoil");
Adafruit_MQTT_Publish humidity = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity");
Adafruit_MQTT_Publish waterIndicator = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/waterindicator");
/****************************** Feeds for Subscribing***************************************/
// Setup a feed called 'slider' for subscribing to changes on the slider
Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider", MQTT_QOS_1);

// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);

/*************************** Sketch Code ************************************/

void slidercallback(double x)
{
  Serial.print("Hey we're in a slider callback, the slider value is: ");
  Serial.println(x);
}

void onoffcallback(char *data, uint16_t len)
{
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);
  String message = String(data);
  message.trim();
  if (message == "ON")
  {
    digitalWrite(D0, LOW);
  }
  if (message == "OFF")
  {
    digitalWrite(D0, HIGH);
  }
}

//*********************************************** SetUp *****************************/
void setup()
{
  // pinMode(D0, OUTPUT);
  pinMode(fc_sensor, INPUT);
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
  Serial.begin(115200);
  // digitalWrite(D0, HIGH);

  // Comenzamos el sensor DHT
  dht.begin();

  delay(10);
  //****************************************** Connect to WiFi access point.
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //********************************************* Callback Functions
  slider.setCallback(slidercallback);
  onoffbutton.setCallback(onoffcallback);

  // Setup MQTT subscription for feed's.
  // mqtt.subscribe(&slider);
  // mqtt.subscribe(&onoffbutton);
}

String message;
int user_num = 0;
int total_distance = 24;
int total_deposito = 14;
int level;
//***************************************************** Loop ********************************/
void loop()
{
  MQTT_connect();
  mqtt.processPackets(10000);
  if (!mqtt.ping())
  { // ping the server to keep the mqtt connection alive
    mqtt.disconnect();
  }
  dhtResponse data = dht11();

  humidity.publish(data.humidity);
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
  Serial.print(" *C ");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C \n");

  return {h, t};
}

void fc28()
{
  delay(1000);
  // put your main code here, to run repeatedly:
  int lectura = analogRead(fc_sensor);

  Serial.print("Valor: ");
  Serial.println(lectura);
}

void ultrasonico()
{
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

  if (level > total_deposito)
  {
    Serial.println("Error de distancia!!");
  }
  else if (level >= 9)
  {
    Serial.println("Deposito lleno!");
  }
  else if (level >= 4)
  {
    Serial.println("Deposito capacidad media!");
  }
  else if (level >= 2)
  {
    Serial.println("Deposito casi vacio!");
  }
  else
  {
    Serial.println("Deposito vacio!");
  }

  delay(2000);
}