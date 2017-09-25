#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "ssid"
const char* password = "password"
//const char* ssid = "ssid"
//const char* password = "password"
//const char* ssid = "ssid"
//const char* password = "password"

const char* host = "2.blkbx.info";

WiFiClient client;
ESP8266WebServer server(80);
MDNSResponder mdns;

int opt[5], c;

int pinCS = 0;
int numberOfHorizontalDisplays = 4;
int numberOfVerticalDisplays = 1;
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);

int wait = 50;

int spacer = 2;
int width = 5 + spacer;

long period;
int offset = 1, refresh = 0;

int hoffset = 1;
//int hoffset = 2;

bool disabled = false;

bool ScrollText(String text, bool checkCheckMode = true);
String getJSON(String path, int port = 80);

String ipToString(IPAddress ip) {
  String s = "";
  for (int i = 0; i < 4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  matrix.setIntensity(13);

  for (int i = 0; i < numberOfHorizontalDisplays; i++) {
    matrix.setRotation(i, 1);
  }

  int x;
  bool color = true;
  while (WiFi.status() != WL_CONNECTED) {
    swipeMatrix(HIGH);
    swipeMatrix(LOW);
  }
  getTime();
  //ScrollText(ipToString(WiFi.localIP()));
  pinMode(A0, INPUT);
  if (MDNS.begin("scroll")) {
    Serial.println("MDNS responder started");
  }
  Serial.println(WiFi.localIP());
  server.on("/scroll", handleScroll);
  server.on("/", handleRoot);
  server.begin();
  opt[1] = 50;
}

int value = 0;

//==============LOOP==============

int dispMode;

void loop() {
  //Serial.println(getJSON("/Downloads/Random/coin/coin.txt"));
  checkMode();
  adjustBrightness();
  if ( dispMode == 1) {
    log("LOOP.Display", "course");
    displayCourse();
  }
  if ( dispMode != 0) {
    displayTime();
  }
  log("LOOP.Display", "time");
  delay(100);
  Serial.println("loop ran.");
  server.handleClient();
}

void handleRoot() {
  for (int i = 0; i < server.args(); i++) {
    opt[server.argName(i).toInt()] = server.arg(i).toInt();
    Serial.println("Opt" + String(server.argName(i)) + ": " + server.arg(i).toInt());
  }
  server.send(200, "text/plain", "ok");
  wait = opt[1];
  dispMode = opt[0];
}

void handleScroll() {
  String text = server.arg(0);
  wait = server.arg(1).toInt();
  server.send(200, "text/plain", "ok");
  ScrollText(text);
  wait = opt[1];
}

String getJSON(String path, int port) {
  if (!client.connect(host, port)) {
    log("getJSON", "failed");
    return "error";
  }
  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    repeatCounter++;
  }
  String result;
  bool keep = false;
  while (client.connected() && client.available()) {
    char c = client.read();
    if (c == '[') keep = true;
    if (keep) result += c;
    if (c == ']') keep = false;
    Serial.print(c);
  }

  Serial.println(result);
  client.stop();
  return result;
}

void displayCourse() {
  String result = getJSON("/Downloads/Random/coin/coin.txt");
  if (result == "error") {
    return;
  }
  DynamicJsonBuffer jsonBuf;
  JsonArray& root = jsonBuf.parseArray(result);
  if (!root.success())
  {
    log("Parsing", "failed");
    return;
  }
  root.prettyPrintTo(Serial);
  for (int i = 0; i < root.size() - 1; i++) {
    String name = root[i]["short"];
    String price = root[i]["price"];
    String perc = root[i]["perc"];
    String shortprice;
    if ( price.indexOf(".") < 4) {
      shortprice = price.substring(0, 5);
    } else {
      shortprice = price.substring(0, price.indexOf("."));
    }
    if (ScrollText(name + " " + shortprice + "$ " + perc + "%")) return;
    matrix.fillScreen(LOW);
  }
}

String date;
int timeResult, timeChecked;
void getTime() {
  if (!client.connect("www.google.com", 80)) {
    Serial.println("connection to google failed");
    return;
  }

  client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: www.google.com\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    repeatCounter++;
  }

  String line;
  client.setNoDelay(false);
  while (client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    line.toUpperCase();
    if (line.startsWith("DATE: ")) {
      date = line.substring(6, 22);
      int h = line.substring(23, 25).toInt();
      int m = line.substring(26, 28).toInt();
      int s = line.substring(29, 31).toInt();
      timeResult = h * 3600 + m * 60 + s;
      timeChecked = millis() / 1000;
    }
  }
  client.stop();
}

void displayTime() {
  for (int i = 0; i < 10; i++) {
    int currentTime = timeResult + millis() / 1000 + timeChecked;
    int h = currentTime / 3600 % 60 + hoffset;
    int m = currentTime / 60 % 60;
    String hh;
    String mm;
    if (h < 10) {
      hh += "0";
    }
    if (m < 10) {
      mm += "0";
    }
    hh += h;
    mm += m;
    if (i % 2) {
      DisplayText(hh + " " + mm);
    } else {
      DisplayText(hh + ":" + mm);
    }
    for (int n = 0; n < 100; n++) {
      if (checkMode()) return;
      delay(10);
    }
  }
}

int potValue, oldPotValue;
bool checkMode() {
  adjustBrightness();
  server.handleClient();
  bool retVal;
  potValue = analogRead(A0);
  if (abs(potValue - oldPotValue) > 10) {
    int i = 0;
    while (i < 50) {
      potValue = analogRead(A0);
      dispMode = potValue / 256;
      //ScrollText("Changed to mode " + String(dispMode) + ".");
      if (!disabled) DisplayText("MODE" + String(dispMode));
      delay(10);
      if (abs(potValue - oldPotValue) < 10) {
        i++;
      } else {
        i = 0;
        Serial.println("reset");
      }
      Serial.println(potValue - oldPotValue);
      log("CM.i", String(i));
      oldPotValue = potValue;
    }
    oldPotValue = potValue;
    log("CM.mode", String(dispMode));
  } else {
    return false;
  }
  if ( dispMode == 0) {
    matrix.setIntensity(1);
    wait = 20;
    ScrollText("Display off.", false);
    wait = opt[1];
    swipeMatrix(LOW);
    disabled = true;
    while (!checkMode()) {
      delay(10);
    }
    disabled = false;
  }
  return true;
}
bool delayAndCheck(int setDelay) {
  for (int n = 0; n < 100; n++) {
    if (checkMode()) return true;
    delay(setDelay / 100);
  }
  return false;
}
void adjustBrightness() {
  if (disabled) return;
  int startTime = 0; // 8
  int stopTime = 24; // 22
  int currentTime = timeResult + millis() / 1000 + timeChecked;
  int h = currentTime / 3600 % 60 + hoffset;
  if (h < startTime || h > stopTime) {
    swipeMatrix(LOW);
    int timeToSleep = 86400 - (currentTime % 86400) + 6 * 3600;
    Serial.println(timeToSleep);
    ScrollText("Sleeping for " + String(timeToSleep / 3600) + " hours.", false);
    for (int i; i < 1e3; i++) delay(timeToSleep);
  }
  matrix.setIntensity(15 * sin(h - startTime * PI / (stopTime - startTime)));
}
void swipeMatrix(int color) {
  for (int x = 0; x <= matrix.width(); x++) {
    //if (checkMode(wait / 2)) return;
    delay(wait / 2);
    for (int y = 0; y < matrix.height(); y++) {
      matrix.drawPixel(x, y, color);
    }
    matrix.write();
  }
}
String oldText;
void DisplayText(String text) {
  if (disabled || text == oldText) return;
  oldText = text;
  matrix.fillScreen(LOW);
  for (int i = 0; i < text.length(); i++) {

    int letter = (matrix.width()) - i * (width - 1);
    int x = (matrix.width() + 1) - letter;
    int y = (matrix.height() - 8) / 2;
    matrix.drawChar(x, y, text[i], HIGH, LOW, 1);
    matrix.write();
  }
}
bool ScrollText (String text, bool checkCheckMode) {
  if (disabled) return true;
  for ( int i = 0 ; i < width * text.length() + matrix.width() - spacer; i++ ) {
    if (refresh == 1) i = 0;
    refresh = 0;
    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2;

    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < text.length() ) {
        matrix.drawChar(x, y, text[letter], HIGH, LOW, 1);
      }
      letter--;
      x -= width;
    }
    matrix.write();
    if (checkCheckMode && checkMode()) return true;
    delay(wait);
  }
  return false;
}
void log(String label, String data) {
  Serial.println(label + ": " + String(data));
}

