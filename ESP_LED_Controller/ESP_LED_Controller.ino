#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "ssid"
const char* password = "password"

//const char* ssid = "ssid"
//const char* password = "password"

//#define PIN            D2
#define PIN            2
#define NUMPIXELS      38
#define DEBUG          1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

ESP8266WebServer server(80);

MDNSResponder mdns;

int opt[5];

void log(String label, int value, int debug = 0);

void setup() {
  pixels.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  int c = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    c++;
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel(c*10, 255));
      pixels.show();
      delay(500/NUMPIXELS);
    }
  }
  Serial.println();
  mdns.begin("leds", WiFi.localIP());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.begin();
  opt[0] = 1; //Mode
  opt[1] = 255; //Brightness
  opt[2] = 0; //Color
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, 0x000000);
  }
  pixels.show();
  pixels.setBrightness(255);
}

void loop() {
  server.handleClient();
  delay(10);
}

void handleRoot() {
  server.send(200, "text/plain", "ok");

  for (int i = 0; i < server.args(); i++) {
    opt[server.argName(i).toInt()] = server.arg(i).toInt();
    log("Opt" + String(server.argName(i)), server.arg(i).toInt());
  }
  if (opt[0] == 1) {
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel(opt[2], opt[1]));
    }
    pixels.show();
  } else if (opt[0] == 2) {
    int dist = opt[2] - opt[3];
    log("dist", dist);
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, Wheel(opt[3] + i * dist / NUMPIXELS, opt[1]));
    }
    pixels.show();
  }
}

uint32_t Wheel(byte WheelPos, int br) {
  WheelPos = 255 - WheelPos % 256;
  if (WheelPos < 85) {
    return pixels.Color((255 - WheelPos * 3) * br / 255, 0, WheelPos * 3 * br / 255);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3 * br / 255, (255 - WheelPos * 3) * br / 255);
  } else {
    WheelPos -= 170;
    return pixels.Color(WheelPos * 3 * br / 255, (255 - WheelPos * 3) * br / 255, 0);
  }
}

void log(String label, int value, int debug) {
  if (DEBUG || debug) {
    Serial.print(label);
    Serial.print(": ");
    Serial.println(value);
  }
}
