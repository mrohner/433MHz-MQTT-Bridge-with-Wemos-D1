/*
  433MHz to MQTT bridge
  Version 1.1     15.6.2018 Added logging facility to Syslog
  Version 1.0     26.3.2018
  by Markus Rohner
  
  Funtion: Reads sensor values via a 433MHz receiver and the RCSwitch library
           The sensor id maps to an MQTT topic and the value is published to an MQTT broker
  
Pin assignments:
 * Wemos D1 mini
 D2: 433MHz Receiver
 
// Reads sensor values via a 433MHz receiver and the RCSwitch library
// The sensor id maps to an MQTT topic and the value is published to an MQTT broker
*/
#include <Arduino.h>
#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Syslog.h>
const bool DEBUG = 0; //Set to 1 for debug on

//
// WIFI and MQTT setup
//
const char* ssid = "***";
const char* password = "***";
const char* mqtt_server = "192.168.178.59";
const char* mqtt_username = "mqtt";
const char* mqtt_password = "***";
char* OutTopic = "domoticz/in"; 

//DOMOTICZ IDX Definitions
// On Switch S1
const int KITCHEN_TOP_IDX = 17;//Arilux
const int KITCHEN_BOARD_IDX = 54;//ESP01
const int STONELAMP_IDX = 55;//Wemos D1 mini
// On S2
const int LIGHT_IDX = 1;//Sonoff Basic
const int CIGAR_IDX = 45;// \Wemos D1 mini
const int SPOT_IDX = 46; // /Wemos D1 mini (same as 45)
// On S3
const int WARDROBE_IDX = 16;//Wemos D1 mini
const int TREPPENLICHT_IDX = 57;//Wemos D1 retired
// On S4
const int SPOT_DINING_ROOM_IDX = 56;//Sonoff Basic
const int OTHER2_IDX = 0;//
const int OTHER3_IDX = 0;//

// Syslog server connection info
#define SYSLOG_SERVER "192.168.178.59"
#define SYSLOG_PORT 514

// This device info
#define DEVICE_HOSTNAME "wemos80"
#define APP_NAME "433MHz-MQTT-Bridge"

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udpClient;

// Create a new syslog instance with LOG_KERN facility
Syslog syslog(udpClient, SYSLOG_SERVER, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, LOG_KERN);

WiFiClient espClient;
PubSubClient client(espClient);
int counter = 0;

//
// 433MHz receiver setup
//
RCSwitch mySwitch = RCSwitch();
#define RECEIVER_INTERRUPT_PIN  4 //D2 Wemos


void setup(){
  Serial.begin(115200);
  mySwitch.enableReceive(RECEIVER_INTERRUPT_PIN);
  if (DEBUG) Serial.println(F("setup start"));
  setup_wifi();
  client.setServer(mqtt_server, 8883);

  //OTA
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("Wemos80-433MQTT_Bridge");
  // No authentication by default
  ArduinoOTA.setPassword((const char *)"***");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) if (DEBUG) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) if (DEBUG) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) if (DEBUG) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) if (DEBUG) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) if (DEBUG) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  WiFiClient WiFiclient;
}


void loop(){
  if (!client.connected()) reconnect();
  if (mySwitch.available()){
    BridgeToMQTT(mySwitch.getReceivedValue());
  }
  ArduinoOTA.handle();
  client.loop();
}


void setup_wifi() {
  WiFi.mode(WIFI_STA); 
  wait(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    wait(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  wait(500);
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("433-MQTT-Bridge",mqtt_username,mqtt_password)) {
      Serial.println("connected");
      counter = 0;
      // Once connected, publish an announcement...
      // ... and resubscribe
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in .3 seconds");
      ++counter;
      if (counter > 180) ESP.reset();
      // Wait 0.3 seconds before retrying
      wait(300);
      ArduinoOTA.handle();
   }
  }
}


void BridgeToMQTT(long message){
  syslog.logf(LOG_INFO, "Received code %d", message);
  switch (message) {
      case 1182164: //S1-1  
        publish(STONELAMP_IDX);
      break;
      case 1182168: //S1-2  
        publish(KITCHEN_BOARD_IDX);
      break;
      case 1182162: //S1-3 
        publish(KITCHEN_TOP_IDX);
      break;
      
      case 3576436: //S2-1 
        publish(LIGHT_IDX);
      break;
      case 3576440: //S2-2  
        publish(CIGAR_IDX);
      break;
      case 3576434: //S2-3  
        publish(SPOT_IDX);
      break;
      
      case 244168: //S3-1 
        publish(WARDROBE_IDX);
      break;
      case 244162: //S3-2  
        publish(TREPPENLICHT_IDX);
      break;
      
      case 11726868: //S4-1 
        publish(SPOT_DINING_ROOM_IDX);
      break;
      case 11726872: //S4-2  
        //publish(IDX);
      break;
      case 11726866: //S4-3  
        //publish(IDX);
      break;
  }
  wait(400);
  mySwitch.resetAvailable();
}


void publish(int idx){ 
  char output[130];
  snprintf_P(output, sizeof(output), PSTR("{\"command\": \"switchlight\",\"idx\":%d,\"switchcmd\": \"Toggle\"}"),idx);
  client.publish(OutTopic,output);
}


void wait (int ms) {
  long current_time = millis();
  long end_time = current_time + ms; 
  while (current_time < end_time) current_time = millis();
  yield();
  ArduinoOTA.handle();
}
