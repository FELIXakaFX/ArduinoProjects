#include <SPI.h>
#include "SSD1306Spi.h"

SSD1306Spi display(D0, D2, D8);

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid = "ssid"
const char* password = "password"

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

void loop() {

  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin("http://2.blkbx.info/Downloads/Random/coin.txt");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request

    if (httpCode > 0) { //Check the returning code
      display.clear();
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload
      display.drawString(0, 0, payload);
      display.display();      
    }

    http.end();   //Close connection

  }

  delay(5000);    //Send a request every 30 seconds

}
