#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

const char* ssid = "ssid"
const char* password = "password"

//const char* ssid = "ssid"
//const char* password = "password"

//const char* ssid = "ssid"
//const char* password = "password"

#define PIN            D2
#define NUMPIXELS      64
#define COLS           8
#define DEBUG          1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

ESP8266WebServer server(80);

MDNSResponder mdns;

WiFiClient client;

int opt[5], c;

bool clear = true;

int col[][5] = {{0xFF0000, 0x00FF00, 0x00FF00, 0x00FF00, 0x00FF00},
  {0xFF0000, 0x0000FF, 0x4400FF, 0xFF0088, 0xFF0044},
  {0xFF0000, 0x00FF00, 0x44FF00, 0xFF8800, 0xFF4400}
};

String date;
int timeResult, timeChecked;

void log(String label, int value, int debug = 0);
void clearMode(int color = 0x000000, int slow = 0);
void apply(int del = 0);
uint32_t Wheel(byte WheelPos, int br = 5);

void setup() {
  pixels.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  int i;
  c = random(0, 256);
  while (WiFi.status() != WL_CONNECTED) {
    clearMode(Wheel(c + i * 40), 5);
    i++;
    Serial.print(".");
  }
  Serial.println();
  mdns.begin("pixels", WiFi.localIP());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  server.on("/", handleRoot);
  server.on("/matrix", handleMatrix);
  server.begin();
  opt[0] = 0; //Mode
  opt[1] = 0; //Msg
  opt[2] = 4; //Delay
  opt[3] = 3; //Brightness
  clearMode(0x000000, 5);
}

void loop() {
  if (opt[0] == 1) {
    clearMode(Wheel(c), 3);
    clearMode(0x000000, 3);
    randomMode();
  } else if (opt[0] == 2) {
    clearMode(Wheel(c), 3);
    clearMode(0x000000, 3);
    snakeMode();
  } else if (opt[0] == 3) {
    clearMode(Wheel(c), 3);
    clearMode(0x000000, 3);
    golMode();
  }
  while (opt[0] == 4) {
    clearMode(Wheel(c), pow(opt[2], 2));
    c+=64/opt[2];
    server.handleClient();
  }
    while (opt[0] == 5) {
    clearMode(Wheel(c));
    c+=1;
    delay(pow(6-opt[2], 2));
    server.handleClient();
  }

  server.handleClient();
  if (opt[1] > 0) {
    notMode();
  }
  delay(100);
  if (clear) {
    clearMode();
  }
}

void handleRoot() {
  for (int i = 0; i < server.args(); i++) {
    opt[server.argName(i).toInt()] = server.arg(i).toInt();
    log("Opt" + String(server.argName(i)), server.arg(i).toInt());
  }
  String html;
  html += "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\"><style>* {font-size: 4vh;color: #d9d9d9;}body {margin: 0px;background: #1c1e21;font-family: sans-serif;}.header, .placeholder {background: #262c35;width: 96%;padding: 2%;}.header {position: fixed;top: 0;left: 0;}.placeholder > span {color: #1c1e21;}.right {text-align: right;}select, button, input {-webkit-appearance: none; -moz-appearance: none;appearance: none;background: #262c35;border: none;width: 100%;height: 100%;}table {width: 100%;}td {background: #262c35;padding: 1%;width: 50%;}</style><body>";
  html += "<form method=\"get\"><table cellspacing=\"5px\">";
  html += "<tr><td colspan=\"2\"><button type=\"submit\" name=\"0\" value=\"" + String((opt[0] + 1) % 4) + "\">Next Mode</button></td></tr>";
  html += "<tr><td colspan=\"2\"><button type=\"submit\" name=\"3\" value=\"" + String((opt[3] + 1) % 5) + "\">Next Brightness</button></td></tr>";
  html += "<tr><td colspan=\"2\"><button type=\"submit\" name=\"2\" value=\"" + String((opt[2] + 1) % 5) + "\">Next Speed</button></td></tr>";

  html += "</form></table></body>";
  server.send(200, "text/html", html);
  clear = true;
}

void handleMatrix() {
  server.send(200, "text/html", "ok");
  for (int i = 0; i < server.args(); i++) {
    int x = server.argName(i).toInt() % COLS;
    int y = server.argName(i).toInt() / COLS;
    pixels.setPixelColor(findPixel(x + 1, y + 1), server.arg(i).toInt());
  }
  pixels.show();
  opt[0] = 0;
  clear = false;
}

int findPixel(int x, int y) {
  if (y % 2 == 0) {
    x = COLS - x;
  } else {
    x--;
  }
  return ((y - 1) * sqrt(NUMPIXELS) + x);
}

void randomMode() {
  while (opt[0] == 1) {
    c++;
    pixels.setPixelColor(random(0, NUMPIXELS), Wheel(c + random(0, 6) * 10));
    pixels.setPixelColor(random(0, NUMPIXELS), 0x000000);
    apply();
  }
  clearMode();
}

void golMode() {
  int map[10][10] = {{0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
  int nmap[10][10];
  for (int x = 0; x < 4; x++) {
    map[random(1, COLS)][random(1, COLS)] = 1;
    delay(10);
  }
  while (opt[0] == 3) {
    c++;
    for (int x = 0 + 1; x < COLS + 1; x++) {
      for (int y = 0 + 1; y < COLS + 1; y++) {
        int nbrs = 0;
        for (int dx = -1; dx < 2; dx++) {
          for (int dy = -1; dy < 2; dy++) {
            nbrs += map[x + dx][y + dy];
          }
        }
        nbrs -= map[x][y];
        if (nbrs > 1 && nbrs < 4) {
          pixels.setPixelColor(findPixel(x, y), Wheel(c + ((nbrs - 2) * 20)));
          nmap[x][y] = 1;
        } else {
          pixels.setPixelColor(findPixel(x, y), 0x000000);
          nmap[x][y] = 0;
        }
      }
    }
    for (int x = 0 + 1; x < COLS + 1; x++) {
      for (int y = 0 + 1; y < COLS + 1; y++) {
        map[x][y] = nmap[x][y];
      }
    }
    apply();
  }
  clearMode();
}

void snakeMode() {
  int mlen = 8;
  int pos[8][2] = {{7, 1}, {6, 1}, {5, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 1}, {0, 1}};
  int fpos[2] = {0, 0};
  int ftype = 0;
  int len = 4;
  int dir = 0, rot[2], fcol, slow = 0;
  while (opt[0] == 2) {
    if (ftype) {
      pixels.setPixelColor(findPixel(fpos[0], fpos[1]), Wheel(c));
    } else {
      pixels.setPixelColor(findPixel(fpos[0], fpos[1]), Wheel(c + len * 10));
    }
    for (int br = 0; br < 5; br++) {
      c++;
      for (int i = 1; i < len; i++) {
        pixels.setPixelColor(findPixel(pos[i][0], pos[i][1]), Wheel(c + i * 10));
      }
      pixels.setPixelColor(findPixel(pos[len][0], pos[len][1]), Wheel(c + len * 10, 4 - br));
      for (int i = len + 1; i < mlen; i++) {
        pixels.setPixelColor(findPixel(pos[i][0], pos[i][1]), 0x000000);
      }
      if (fpos[0] == 0 || pos[0][0] == fpos[0] && pos[0][1] == fpos[1]) {
        pixels.setPixelColor(findPixel(pos[0][0], pos[0][1]), Wheel(c));
        if (ftype) {
          if (slow == 0) slow = 10;
        } else {
          pixels.setPixelColor(findPixel(pos[len + 1][0], pos[len + 1][1]), Wheel(c + len * 10, 4 - br));
          pixels.setPixelColor(findPixel(pos[len][0], pos[len][1]), Wheel(c + len * 10, 4 - br));
        }
      } else {
        pixels.setPixelColor(findPixel(pos[0][0], pos[0][1]), Wheel(c, br + 1));
      }
      if (slow > 0) {
        pixels.setPixelColor(findPixel(pos[len][0], pos[len][1]), Wheel(c + len * 10, slow / 2 - 0.5));
        log("slow", slow);
        slow--;
      }
      apply(pow(2, opt[2] + 4));
    }
    if (fpos[0] == 0 || pos[0][0] == fpos[0] && pos[0][1] == fpos[1]) {
      while (true) {
        ftype = random(0, 2);
        if ((ftype == 1 && len < mlen - 1) || (ftype == 0 && len > 2)) break;
      }
      while (true) {
        for (int i = 0; i <= 2; i++) {
          fpos[i] = random(1, COLS);
        }
        if ((fpos[0] - pos[0][0] > 1 || pos[0][0] - fpos[0] > 1) && (fpos[1] - pos[0][1] > 1 || pos[0][1] - fpos[1] > 1)) break;
        log("xfpos", fpos[0]);
        log("yfpos", fpos[1]);
        log("ftype", ftype);
        log("length", len);
      }
    }

    rot[0] = 0;
    rot[1] = 0;

    if (dir < 2 && pos[0][dir % 2] >= fpos[dir % 2]) {
      rot[0] = 1;
      rot[1] = -1;
    } else if (dir >= 2 && pos[0][dir % 2] <= fpos[dir % 2]) {
      rot[0] = -1;
      rot[1] = 1;
    }
    if (pos[0][(dir + 1) % 2] >= fpos[(dir + 1) % 2]) {
      dir = dir + 4 - rot[dir % 2];
    } else {
      dir = dir + 4 + rot[dir % 2];
    }
    dir = dir % 4;
    for (int i = 1; i < mlen; i++) {
      pos[mlen - i][0] = pos[mlen - 1 - i][0];
      pos[mlen - i][1] = pos[mlen - 1 - i][1];
    }
    if (dir < 2) {
      pos[0][dir % 2]++;
      pos[0][dir % 2] = (pos[0][dir % 2] - 1) % COLS + 1;
    } else {
      pos[0][dir % 2]--;
      pos[0][dir % 2] = ((pos[0][dir % 2] - 1) + COLS) % COLS + 1;
    }
    /*log("Dir", dir);
      log("xpos", pos[0][0]);
      log("ypos", pos[0][1]);*/
  }
  clearMode();
}

void notMode() {
  if (timeResult + millis() / 1000 + timeChecked) return;
  char icns[][8][8] = {
    { // WhatsApp
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 2, 2, 2, 2, 0, 0},
      {0, 2, 2, 1, 1, 1, 2, 0},
      {0, 2, 1, 2, 1, 1, 2, 0},
      {0, 2, 1, 1, 2, 1, 2, 0},
      {0, 2, 1, 1, 1, 2, 2, 0},
      {0, 2, 2, 2, 2, 2, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Youtube
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 2, 2, 2, 2, 1, 0},
      {0, 2, 2, 1, 1, 2, 2, 0},
      {0, 2, 2, 1, 1, 2, 2, 0},
      {0, 1, 2, 2, 2, 2, 1, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Mail
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 1, 2, 2, 2, 2, 1, 0},
      {0, 2, 1, 2, 2, 1, 2, 0},
      {0, 2, 2, 1, 1, 2, 2, 0},
      {0, 2, 2, 2, 2, 2, 2, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Battery
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 2, 2, 2, 2, 2, 0, 0},
      {0, 2, 1, 0, 0, 2, 2, 0},
      {0, 2, 1, 0, 0, 2, 2, 0},
      {0, 2, 2, 2, 2, 2, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    },
    { // Warning 1
      {1, 1, 1, 0, 0, 1, 1, 1},
      {1, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 1},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {1, 0, 0, 0, 0, 0, 0, 1},
      {1, 0, 0, 0, 0, 0, 0, 1},
      {1, 1, 1, 0, 0, 1, 1, 1}
    },
    { // Warning 2
      {1, 1, 1, 2, 2, 1, 1, 1},
      {1, 2, 2, 0, 0, 2, 2, 1},
      {1, 2, 0, 0, 0, 0, 2, 1},
      {2, 0, 0, 0, 0, 0, 0, 2},
      {2, 0, 0, 0, 0, 0, 0, 2},
      {1, 2, 0, 0, 0, 0, 2, 1},
      {1, 2, 2, 0, 0, 2, 2, 1},
      {1, 1, 1, 2, 2, 1, 1, 1}
    },
    { // Blank
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0},
      {0, 0, 0, 0, 0, 0, 0, 0}
    }
  };
  int icol[][2][3] = {{{0x00FF00, 0x44FF44, 0xFFFFFF}, {0x000000, 0x004400, 0x00FF00}},
    {{0xFFFFFF, 0xFF4444, 0xFF0000}, {0x000000, 0x220000, 0xFF0000}},
    {{0x0000FF, 0x4444FF, 0xFFFFFF}, {0x000000, 0x000022, 0x0000FF}},
    {{0x000000, 0xFF0000, 0xFFFFFF}, {0x000000, 0xFF0000, 0xFFFFFF}},
    {{0x000000, 0xFF0000, 0xFFFFFF}, {0x000000, 0x880000, 0xFFFFFF}},
    {{0x000000, 0xFF0000, 0x880000}, {0x440000, 0xFF0000, 0x880000}}
  };
  int i = (opt[1] - 1) % 10;
  int n = ((opt[1] - 1) - i) / 10;
  log("i", i);
  log("n", n);
  for (int z = 0; z < 900; z++) {
    pixels.setBrightness(sin(z % 180 * PI / 180) * 255);
    for (int y = 0; y < COLS; y++) {
      for (int x = 0; x < COLS; x++) {
        pixels.setPixelColor(findPixel(x + 1, y + 1), icol[i][n][icns[i][y][x]]);
      }
    }
    delay(10);
    pixels.show();
  }
  clearMode();
  pixels.setBrightness(pow(2, opt[3] + 4) - 1);
  pixels.show();
  delay(pow(2, opt[2] + 4));
  server.handleClient();
  opt[1] = 0;
}

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

//======================FUNCTIONS======================

int color(int col, int max) {
  int color[3];
  int mtrx[27][3];
  for (int i = 0; i < 28; i++) {
    int tmp = i;
    for (int j = 0; j < 3; j++) {
      mtrx[i][j] = tmp % 3;
      tmp = tmp / 3;
    }
  }
  color[0] = 0;
  color[1] = col * 255 / max;
  color[2] = (max - col) * 255 / max;
  return pixels.Color(color[mtrx[opt[1]][0]], color[mtrx[opt[1]][1]], color[mtrx[opt[1]][2]]);
}

uint32_t Wheel(byte WheelPos, int br) {
  WheelPos = 255 - WheelPos % 256;
  if (WheelPos < 85) {
    return pixels.Color((255 - WheelPos * 3) * br / 5, 0, WheelPos * 3 * br / 5);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3 * br / 5, (255 - WheelPos * 3) * br / 5);
  } else {
    WheelPos -= 170;
    return pixels.Color(WheelPos * 3 * br / 5, (255 - WheelPos * 3) * br / 5, 0);
  }
}

void clearMode(int color, int slow) {
  for (int y = 0 + 1; y < COLS + 1; y++) {
    for (int x = 0 + 1; x < COLS + 1; x++) {
      pixels.setPixelColor(findPixel(x, y), color);
      if (slow > 0) {
        delay(slow);
        pixels.show();
      }
    }
  }
  pixels.show();
}

void apply(int del) {
  if (del == 0) {
    del = pow(2, opt[2] + 4);
  }
  pixels.setBrightness(pow(2, opt[3] + 4) - 1);
  pixels.show();
  delay(del);
  server.handleClient();
  if (opt[1] > 0) {
    notMode();
  }
}

void log(String label, int value, int debug) {
  if (DEBUG || debug) {
    Serial.print(label);
    Serial.print(": ");
    Serial.println(value);
  }
}
