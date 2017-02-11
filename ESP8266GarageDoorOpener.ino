#include "./libraries/RFControl/RFControl.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#ifndef RF_CONTROL_VARDUINO
#include "libraries/RFControl/arduino_functions.h"
#endif

extern "C" {
  #include "user_interface.h"
}

#include "SSID_PASSWORD.h"

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

ESP8266WebServer server(80);
const bool DEBUG_HTTP = 0;
long commandsSent = 0;
const int pin = 2;

#define TITEL "Garagenschalter"

void initWiFi() {
  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
}




void loop() {
  server.handleClient();
  wifi_set_sleep_type(LIGHT_SLEEP_T);
  delay(200);
}

String head() {
  return "<!DOCTYPE html>\r\n<html>\r\n<head>\r\n<meta content=\"text/html; charset=ISO-8859-1\" http-equiv=\"content-type\">\r\n<title>" TITEL "</title>\r\n</hread>\r\n<body>\r\n";
}

String tail() {
  return "</body>\r\n</html>\r\n";
}

void sendResult(String &resp) {
  server.send(200, "text/html", resp);
}

String statusBody() {
  String res = "<p>\r\n";
  res += "Commands sent: ";
  res += commandsSent;
  res += "\r\n</p>\r\n";
  return res;
}

String buttonBody() {
  return "<form action=\"/\" method=\"post\"><button type=\"submit\">Switch</button></form>\r\n";
}

void handleIndex() {
  sendResult(head() + statusBody() + buttonBody() + tail());
}

void handleSwitch() {
  garageSend();
  sendResult(head() + "<p>sent command</p>\r\n" + statusBody() + buttonBody() + tail());
}

void initServer() {
  // Start the server
  server.on("/", HTTP_GET, handleIndex);
  server.on("/", HTTP_POST, handleSwitch);
  server.on("/switch", HTTP_GET, handleSwitch);
  server.begin();
  Serial.print("Server started on ");
  Serial.println(WiFi.localIP());
}


void garageSend() {
  if (!DEBUG_HTTP) {
    int pin = 2;
    unsigned long int buckets[] = {404, 1184, 20508};
    char pulses[] = "001010101011010101001100110100101011001100110100101010101010101102";
    unsigned int repeats = 30;
    RFControl::sendByCompressedTimings(pin, buckets, pulses, repeats);  
  }
  commandsSent++;
}

void init433() {
  hw_pinMode(pin, OUTPUT);
  hw_digitalWrite(pin, LOW);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("initializing 433MHz transmitter\r\n");
  init433();
  Serial.println("initializing WIFI\r\n");
  initWiFi();
  Serial.println("initializing server\r\n");
  initServer();
  Serial.println("ready for commands\r\n");
}


