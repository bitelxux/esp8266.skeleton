
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
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>

// This is for each variable to use it's real size 
// when stored in the EPROM
#pragma pack(push, 1)


#define ID "skelethon"

#define LED BUILTIN_LED

// prototipes
void handleOTA();
void blinkLed();
void connectIfNeeded();
void imAlive();

const char* ssid = "xxx";
const char* password = "xxx";

const char* log_server = "http://192.168.1.162:8888";
char buffer[100];

unsigned int tConnect = millis();
unsigned long tLastConnectionAttempt = 0;

struct
{
    boolean enabled;
    unsigned long timer;
    unsigned long lastRun;
    void (*function)();
    char* functionName;
} TIMERS[] = {
  { true, 1*1000, 0, &blinkLed, "blinkLed" },
  { true, 60*1000, 0, &imAlive, "imAlive" },
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

bool send(String what){

  if (WiFi.status() != WL_CONNECTED){
    return false;
  }
  
  bool result;
  WiFiClient client;
  HTTPClient http;
  //Serial.print("sending ");
  //Serial.println(what.c_str());
  http.begin(client, what.c_str());
  int httpResponseCode = http.GET();

  if (httpResponseCode == 200){
        result = true;
      }
      else {
        Serial.print("[send] Error code: ");
        Serial.println(httpResponseCode);
        result = false;
      }
      // Free resources
      http.end();

      return result;
}


void attendTimers(){    
  byte NUM_TIMERS = (sizeof(TIMERS) / sizeof(TIMERS[0]));
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
    IPAddress ip = WiFi.localIP();
    sprintf(buffer, "Connected to %s with IP %d.%d.%d.%d", ssid, ip[0], ip[1], ip[2], ip[3]); 
    Serial.println(buffer);
    log(buffer);
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

void log(char* msg){
  char buffer[100];
  sprintf(buffer, "%s/log/[%s] %s", log_server, ID, msg);
  String toSend = buffer;
  toSend.replace(" ", "%20");
  send(toSend);
}

void imAlive(){
  static long cont = 0;
  char msg[50];
  sprintf(msg, "I'm alive! [%d]", ++cont);
  log(msg);
}

void loop() {  
  attendTimers();
  delay(20);
}
