#include <SPI.h>
#include "SSD1306Spi.h"

SSD1306Spi display(D0, D2, D8);

#include <ESP8266WiFi.h>

const char* ssid = "ssid"
const char* password = "password"

const char* host = "2.blkbx.info";

String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void setup() {
  Serial.begin(115200);
  delay(10);
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Connecting to " + String(ssid));
  display.display();
  
  WiFi.begin(ssid, password);

  String out;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    out += '.';
    display.drawString(0, 10, out);
    display.display();
  }

  display.clear();
  display.drawString(0, 0, "WiFi connected\nIP address: \n"+ipToString(WiFi.localIP()));
  display.display();
}

int value = 0;

void loop() {
  delay(5000);
  ++value;

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // This will send the request to the server
  client.print(String("GET ") + "/Downloads/Random/coin.txt" + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  delay(10);
  
  display.clear();
  int ln = 0;
  while(client.available()){
    String line = client.readStringUntil('\r');
    //display.drawString(0, 0, line);
    if(ln = 12) {
    display.drawString(0, 0, line);
    Serial.print(line);
    }
    ln++;
    }
  display.display();
}
