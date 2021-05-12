#include <Arduino.h>
#include <SPI.h>
#include <WakeOnLan.h>
#include <ESP32Ping.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <HTTPCLient.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecure.h>
#include "config.h"

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library

#define TFT_GREY 0x5AEB // New colour
const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
WiFiUDP UDP;
WakeOnLan WOL(UDP);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void wol()
{
  const char *MACAddress = MAC_ADDRESS;

  WOL.sendMagicPacket(MACAddress); // Send Wake On Lan packet with the above MAC address. Default to port 9.
                                   // WOL.sendMagicPacket(MACAddress, 7); // Change the port number
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
    tft.println("Connecting to WiFi..");
    tft.setCursor(0, 0);
    delay(500);
    tft.fillScreen(TFT_BLACK);
  }
  tft.println("Connected to the\nWiFi network");
}
IPAddress ip(192, 168, 1, 100);
void loop()
{

  // Set the font colour to be white with a black background, set text size multiplier to 1
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
    tft.setCursor(0, 40);
    tft.println("Server is running");
    delay(2000);
  }
  else
  {
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0);
    tft.print("Server ");
    tft.print(ip);
    tft.setCursor(0, 20);
    tft.println("is not running");
    tft.setCursor(0, 40);
    tft.println("Sending WOL Package");
    for (int i = 0; i < 3; i++)
    {
      wol();
    }
    bot.sendMessage(CHAT_ID, "Warning, Server is not reachable, trying to start it with WOL magic packet 💥💥💥");
    delay(60000);
  }
}