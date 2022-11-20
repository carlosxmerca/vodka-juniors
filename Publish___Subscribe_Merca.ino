#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>      // incluye libreria bus SPI
#include <MFRC522.h>      // incluye libreria especifica para MFRC522

#define RST_PIN  D1      // constante para referenciar pin de reset
#define SS_PIN  D2      // constante para referenciar pin de slave select

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "ARTEFACTOS"
#define WLAN_PASS       "ARTEFACTOS"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "carlosxmerca"
#define AIO_KEY         "aio_eAyM99DuWZ9R2Wqg1HRBsMLl648H"


MFRC522 mfrc522(SS_PIN, RST_PIN); // crea objeto mfrc522 enviando pines de slave select y reset

byte LecturaUID[4];         // crea array para almacenar el UID leido
byte Usuario1[4]= {0xC6, 0xD4, 0xA5, 0x2B} ;    // UID de tarjeta leido en programa 1
byte Usuario2[4]= {0x29, 0xEA, 0x2D, 0xC2} ;    // UID de llavero leido en programa 1

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiClientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds for Publishing***************************************/
// Setup a feed called 'photocell' for publishing.
Adafruit_MQTT_Publish user = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/usuario");

/****************************** Feeds for Subscribing***************************************/
// Setup a feed called 'slider' for subscribing to changes on the slider
Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider", MQTT_QOS_1);

// Setup a feed called 'onoff' for subscribing to changes to the button
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);

/*************************** Sketch Code ************************************/

void slidercallback(double x) {
  Serial.print("Hey we're in a slider callback, the slider value is: ");
  Serial.println(x);
}

void onoffcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);
  String message = String(data);
      message.trim();
      if (message == "ON") {digitalWrite(D0, LOW);}
      if (message == "OFF") {digitalWrite(D0, HIGH);}}


//*********************************************** SetUp *****************************/
void setup() {
  // pinMode(D0, OUTPUT);
  Serial.begin(115200);
  // digitalWrite(D0, HIGH);
  SPI.begin();        // inicializa bus SPI
  mfrc522.PCD_Init();     // inicializa modulo lector
  
  delay(10);
  //****************************************** Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());
  //********************************************* Callback Functions
  slider.setCallback(slidercallback);
  onoffbutton.setCallback(onoffcallback);
  
  // Setup MQTT subscription for feed's.
  // mqtt.subscribe(&slider);
  // mqtt.subscribe(&onoffbutton);

}


String message;
int user_num = 0;
//***************************************************** Loop ********************************/
void loop() {
  MQTT_connect();
  mqtt.processPackets(10000);
  if(! mqtt.ping()) {   // ping the server to keep the mqtt connection alive
    mqtt.disconnect();}

  if ( ! mfrc522.PICC_IsNewCardPresent())   // si no hay una tarjeta presente
    return;           // retorna al loop esperando por una tarjeta
  
  if ( ! mfrc522.PICC_ReadCardSerial())     // si no puede obtener datos de la tarjeta
    return;           // retorna al loop esperando por otra tarjeta
    
    Serial.print("UID:");       // muestra texto UID:
    for (byte i = 0; i < mfrc522.uid.size; i++) { // bucle recorre de a un byte por vez el UID
      if (mfrc522.uid.uidByte[i] < 0x10){   // si el byte leido es menor a 0x10
        Serial.print(" 0");       // imprime espacio en blanco y numero cero
      }
      else {           // sino
          Serial.print(" ");        // imprime un espacio en blanco
      }
      Serial.print(mfrc522.uid.uidByte[i], HEX);    // imprime el byte del UID leido en hexadecimal
      LecturaUID[i]=mfrc522.uid.uidByte[i];     // almacena en array el byte del UID leido      
    }
          
    Serial.print("\t");         // imprime un espacio de tabulacion             
    if(comparaUID(LecturaUID, Usuario1)) {   // llama a funcion comparaUID con Usuario1
      message = "Bienvenido Carlos!"; // si retorna verdadero muestra texto bienvenida
      user_num = 1;
    } else if(comparaUID(LecturaUID, Usuario2)) { // llama a funcion comparaUID con Usuario2
      message = "Bienvenido Eduardo!"; // si retorna verdadero muestra texto bienvenida
      user_num = 2; 
    }else {         // si retorna falso
      message = "Me estan hackeando :0"; // muestra texto equivalente a acceso denegado
      user_num = 3;
    }
                
    Serial.println(message);           
    mfrc522.PICC_HaltA();     // detiene comunicacion con tarjeta
                  
    // Now we can publish stuff!
    Serial.print(F("\nSending user info"));
    // Serial.print(x);
    Serial.print("...");
    if (! user.publish(user_num)) {
      Serial.println(F("Failed"));
    } else {
      Serial.println(F("OK!"));
    }
}

boolean comparaUID(byte lectura[],byte usuario[]) // funcion comparaUID
{
  for (byte i=0; i < mfrc522.uid.size; i++){    // bucle recorre de a un byte por vez el UID
  if(lectura[i] != usuario[i])        // si byte de UID leido es distinto a usuario
    return(false);          // retorna falso
  }
  return(true);           // si los 4 bytes coinciden retorna verdadero
}

// Function to connect and reconnect as necessary to the MQTT server.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;}
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 10 seconds...");
       mqtt.disconnect();
       delay(10000);  // wait 10 seconds
       retries--;
       if (retries == 0) {       // basically die and wait for WDT to reset me
         while (1);}}
  Serial.println("MQTT Connected!");
}
