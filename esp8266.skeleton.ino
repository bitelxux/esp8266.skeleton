
/* Testest with
 -  Arduino IDE 1.8.19
    - Arduino AVR board 1.8.3
    - ESP8266 3.0.2

https://arduino.esp8266.com/stable/package_esp8266com_index.json
board: NodeMCU1.0 (ESP-12E Module)    
*/


/*
D0 = GPIO16;
D1 = GPIO5;
D2 = GPIO4;
D3 = GPIO0;
D4 = GPIO2;
D5 = GPIO14;
D6 = GPIO12;
D7 = GPIO13;
D8 = GPIO15;
D9 = GPIO3;
D10 = GPIO1;
LED_BUILTIN = GPIO16 (auxiliary constant for the board LED, not a board pin);
*/

//Libraries
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

// This is for each variable to use it's real size 
// when stored in the EPROM
#pragma pack(push, 1)

#define LED BUILTIN_LED

// prototipes
void handleOTA();
void blinkLed();
void connectIfNeeded();

const char* ssid = "SSID";
const char* password = "xxxxx";

unsigned int tConnect = millis();
unsigned long tLastConnectionAttempt = 0;

// Timers
#define NUM_TIMERS 3 // As I can't find an easy way to get the number
                     // of elements in an array
struct
{
    boolean enabled;
    unsigned long timer;
    unsigned long lastRun;
    void (*function)();
    char* functionName;
} TIMERS[] = {
  { true, 1*1000, 0, &blinkLed, "blinkLed" },
  { true, 1*1000, 0, &handleOTA, "handleOTA" },
  { true, 5*1000, 0, &connectIfNeeded, "connectIfNeeded" },  
};

void setup() {
  pinMode(LED, OUTPUT);  
  Serial.begin(115200);
}

void handleOTA(){
  ArduinoOTA.handle();
}

void attendTimers(){    
  for (int i=0; i<NUM_TIMERS; i++){
    if (TIMERS[i].enabled && millis() - TIMERS[i].lastRun >= TIMERS[i].timer) {
      TIMERS[i].function();
      TIMERS[i].lastRun = millis();
    }
  }
}

void connect(){
  Serial.println("");
  Serial.println("Connecting");

  WiFi.begin(ssid, password);

  tLastConnectionAttempt = millis();
  tConnect =  millis();
  while(WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED, !digitalRead(LED));
    delay(100);
    if ((millis() - tConnect) > 500){
      Serial.print(".");
      tConnect = millis();
    }

    // If it doesn't connect, let the thing continue
    // in the case that in a previous connection epochTime was
    // initizalized, it will store readings for future send
    if (millis() - tLastConnectionAttempt >= 30000L){      
      break;
    }    
  }

  Serial.println("");

  if (WiFi.status() == WL_CONNECTED) { 
    Serial.println("");
    Serial.print("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());  
    ArduinoOTA.begin();
  }
  else
  {
    Serial.println("Failed to connect");
  }

  tLastConnectionAttempt = millis();
}

void blinkLed(){
  digitalWrite(LED, !digitalRead(LED));
}

void connectIfNeeded(){
  // If millis() < 30000L is the first boot so it will try to connect
  // for further attempts it will try with spaces of 60 seconds

  if (WiFi.status() != WL_CONNECTED && (millis() < 30000L || millis() - tLastConnectionAttempt > 60000L)){
    Serial.println("Trying to connect");
    connect();
  }  
}

void loop() {  
  attendTimers();
  delay(20);
}
