#include <ESP8266WiFi.h>
#include <string>
#include <sstream>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

const char* ssid = "CLARO_CD928A";
const char* password = "Bca2cca84F";

//Your Domain name with URL path or IP address with path
String serverName = "https://vodkaapi.fly.dev/";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 15000;

// Pines
int bomba = D1;

// Global var
int day, hour;
int Days[7] = {2,2,2,2,2,2,2}; 
String response;
struct responseDH{
    int day;
    int hour;
};

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
        
        Serial.println(hour);
        Serial.println(day);

        // Watering - routine
        wateringRoutine(day, hour);
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
    if((hour == 5 || hour == 17) && Days[day] != 0) {
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
