/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-post-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  
  Code compatible with ESP8266 Boards Version 3.0.0 or above 
  (see in Tools > Boards > Boards Manager > ESP8266)
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

const char* ssid = "CLARO_CD928A";
const char* password = "Bca2cca84F";

//Your Domain name with URL path or IP address with path
String serverName = "https://api.twilio.com/";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 10000;

void setup() {
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
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {
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

      String serverPath = serverName + "2010-04-01/Accounts/AC35f55d787a8a836461c3c932ca9ce61d/Messages.json";
      
      // Your Domain name with URL path or IP address with path
      http.begin(*client, serverPath.c_str());
      
      http.addHeader("Content-Type", "application/json");

      // If you need Node-RED/server authentication, insert user and password below
      http.setAuthorization("AC35f55d787a8a836461c3c932ca9ce61d", "5b6fcd2abc02fa9ec8522f352f173c31");
         
      //Instead of TEXT as Payload, can be JSON as Paylaod   
      String payload = "{\r\n\"To\":\"whatsapp:+50370398239\",\r\n\"From\":\"whatsapp:+14155238886\",\r\n\"Body\":\"Hello this is a test\",\r\n\"filter\": {\"FTX\": \"0001\"}\r\n}}"; 

      // Send HTTP GET request
      // int httpResponseCode = http.GET();
      int httpResponseCode = http.POST(payload);
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
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