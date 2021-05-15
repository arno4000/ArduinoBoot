#include <Arduino.h>
#include <SPI.h>
#include <WakeOnLan.h>
#include <ESP32Ping.h>
#include <TFT_eSPI.h> 
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI(); 

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
const char *mqttServer = MQTT_SERVER;
const int mqttPort = 1883;
WiFiUDP UDP;
WakeOnLan WOL(UDP);
WiFiClientSecure secured_client;
WiFiClient wificlient;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
PubSubClient client(wificlient);

void wol()
{
  const char *MACAddress = MAC_ADDRESS;

  WOL.sendMagicPacket(MACAddress); 
}

void onMessage(String &topic, String &payload){
  Serial.println(topic+" "+payload);
}
void setup(void)
{
  tft.init();
  tft.setRotation(3);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED)
  {
    tft.setCursor(0, 0);
    tft.println("Connecting to WiFi..");
    delay(500);
    tft.fillScreen(TFT_BLACK);
  }
  tft.setCursor(0,0);
  tft.println("Connected to the\nWiFi network");
  tft.print("IP: ");
  tft.println(WiFi.localIP());
  delay(2000);

  client.setServer(mqttServer, mqttPort);
  while(!client.connected()){
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0,0);
    tft.println("Connecting to MQTT");
    if(client.connect("ArduinoBoot", MQTT_USER, MQTT_PASSWORD)){
      tft.println("connected");
    } else{
      tft.println("ERROR");
      tft.println("SEE SERIAL LOG");
      Serial.println(client.state());
      delay(500);
    }
  }


}
IPAddress ip(192, 168, 1, 100);
void loop()
{

  if (WiFi.status() != WL_CONNECTED)
  {
    ESP.restart();
  }

  bool ping = Ping.ping(ip, 3);
  if (ping)
  {
    String pingTime = String(Ping.averageTime());
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.println(pingTime + " ms");
    tft.println("Server is running");
    delay(2000);
    const char* message = "192.168.1.100: running";
    client.publish("kekw", message);
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.print("Server ");
    tft.print(ip);
    tft.println("is not running");
    tft.println("Sending WOL Package");
    for (int i = 0; i < 3; i++)
    {
      wol();
    }
    bot.sendMessage(CHAT_ID, "Warning, Server is not reachable, trying to start it with WOL magic packet 💥💥💥");
    const char* message = "192.168.1.100: down";
    client.publish("kekw", message);
    delay(10000);
  }
}