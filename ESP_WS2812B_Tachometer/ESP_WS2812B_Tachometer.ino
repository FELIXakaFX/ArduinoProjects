#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

const int LED_PIN = D8;
const int LED_COUNT = 11;

const int REED_PIN = D7;   // Reed switch
const int radius = 2236;  // Actually this is the circumeference (in mm)

int kmh, maxkmh, mode, distance;

long odistance;

long oelapsed = 0;

int dpos = 3;
int sopts = 4;
int opt[7];
bool batterylow = false;


Adafruit_NeoPixel leds =
  Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void display(int kmh, int clear = 0);
void showbar(int dots, int fg, int dt, int bg = 0x000000);

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  pinMode(REED_PIN, INPUT_PULLUP);
  pinMode(0, INPUT);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("tacho", "12344321");
  leds.begin();
  server.on("/", handleRoot);
  server.on("/json", handleJSON);
  server.begin();
  opt[sopts] = 0;
  opt[sopts + 2] = 0;
  odistance = readDistance();
  for (int j = 0; j < sopts; j++) {
    opt[j] = EEPROM.read(dpos + j);
  }
  for (int i = 0; i < LED_COUNT * 5; i++) {
    delay(20);
    display(i, 1);
  }
  for (int i = 0; i < LED_COUNT; i++) {
    delay(20);
    leds.setPixelColor(i, 0x000000);
    leds.show();
  }
}

void loop() {
  while (digitalRead (REED_PIN) == HIGH) {
    server.handleClient();
    delay(1);
  }
  long start = millis ();
  while (digitalRead (REED_PIN) == LOW) {
    delay(1);
  }
  delay(50);
  while (digitalRead (REED_PIN) == HIGH) {
    server.handleClient();
    delay(1);
  }
  long finished = millis ();
  long elapsed = finished - start;
  if (oelapsed == 0 || (elapsed > 0.5 * oelapsed && elapsed < 2 * oelapsed)) {
    kmh = 3.6 * radius / elapsed;
  }
  oelapsed = elapsed;
  distance++;
  Serial.print(elapsed);
  Serial.print("ms ");
  Serial.print(kmh);
  Serial.println("kmh ");
  display(kmh);
  if (kmh > maxkmh) {
    maxkmh = kmh;
  }
}

void display(int kmh, int clear) {
  if (opt[0] == 0) {
    int dots, bars, color;

    int colors[][6] = {{0x000000, 0x0000FF, 0x00FF88, 0x00FF00, 0xFF8800, 0xFF0000},
      {0x000000, 0x88FF00, 0xFFFF00, 0xFFBB00, 0xFF4400, 0xFF0000},
      {0x000000, 0x0000FF, 0x8800FF, 0xFF0088, 0xFF0044, 0xFF0000}
    };
    if (kmh <= LED_COUNT && distance % 10) {
      if (readBatteryPercentage() < 10) batterylow = true;
    }
    if (kmh <= LED_COUNT && distance % 2 && batterylow) {
      dots = LED_COUNT;
      color = 0xFF0000;
    } else if (opt[sopts] > 0 && kmh <= LED_COUNT) {
      dots = distance * radius / opt[sopts] / 100;
      color = 0xFFFFFF;
    } else {
      dots = (kmh - 1) % LED_COUNT;
      bars = (kmh - 1) / LED_COUNT;
      color = colors[opt[1]][bars + 1];
    }
    if (clear == 0) {
      showbar(dots, color, 0xFFFFFF);
    } else {
      showbar(dots, color, 0xFFFFFF, colors[opt[1]][bars]);
    }
  } else if (opt[0] == 1) {
  }
}

void showbar(int dots, int fg, int dt, int bg) {
  for (int i = 0; i < dots; i++) {
    leds.setPixelColor(i, fg);
  }
  if (opt[3] == 1) {
    leds.setPixelColor(dots, dt);
  } else {
    leds.setPixelColor(dots, fg);
  }
  for (int i = LED_COUNT; i > dots; i--) {
    leds.setPixelColor(i, bg);
  }
  leds.show();
}

void handleRoot() {
  for (int i = 0; i < server.args(); i++) {
    opt[server.argName(i).toInt()] = server.arg(i).toInt();
    Serial.print(server.argName(i));
    Serial.print("=");
    Serial.println(server.arg(i));
  }
  if (opt[sopts + 1] == 1) {
    for (int j = 0; j < sopts; j++) {
      EEPROM.write(dpos + j, opt[j]);
    }
    EEPROM.commit();
  } else if (opt[sopts + 1] == 2) {
    writeDistance(odistance + distance * radius / 1000);
  }
  leds.setBrightness((opt[2] + 1) * 25);
  display(kmh);
  int battery = readBatteryPercentage();
  String bg = "fff";
  if (battery < 20) bg = "f00";
  String html;
  html += "<meta name=\"apple-mobile-web-app-capable\" content=\"yes\"><style>* {font-size: 4vh;color: #d9d9d9;}body {margin: 0px;background: #1c1e21;font-family: sans-serif;}.header, .placeholder {background: #262c35;width: 96%;padding: 2%;}progress, .header {position: fixed;top: 0;left: 0;}.placeholder > span {color: #1c1e21;}.right {text-align: right;}select, button, input {-webkit-appearance: none; -moz-appearance: none;appearance: none;background: #262c35;border: none;width: 100%;height: 100%;}table {width: 100%;}td {background: #262c35;padding: 1%;width: 50%;}</style><body>";
  html += "<script>(function(a,b,c){if(c in b&&b[c]){var d,e=a.location,f=/^(a|html)$/i;a.addEventListener(\"click\",function(a){d=a.target;while(!f.test(d.nodeName))d=d.parentNode;\"href\"in d&&(chref=d.href).replace(e.href,\"\").indexOf(\"#\")&&(!/^[a-z\+\.\-]+:/i.test(chref)||chref.indexOf(e.protocol+\"//\"+e.host)===0)&&(a.preventDefault(),e.href=d.href)},!1)}})(document,window.navigator,\"standalone\");</script>";
  html += "<style>progress{-webkit-appearance: none;appearance: none;width: 100%;height: 5px;z-index: 1;}progress::-webkit-progress-value{background: #" + bg + ";}</style>";
  html += "<progress max=\"100\" value=\"" + String(battery) + "\"></progress>";
  if (opt[sopts + 2] == 0) {
    html += "<a href=\"?" + String(sopts + 2) + "=1\"><div class=\"header\"><span>" + String(distance * radius / 1000) + "m</span><span style=\"float: right;text-align: right;\">" + String(maxkmh) + "km/h</span></div></a>";
    html += "<div class=\"placeholder\"><span style=\"text-align: center;\">" + String(millis() / 1000 % 86400 / 3600) + ":" + String(millis() / 1000 % 3600 / 60) + ":" + String(millis() / 1000 % 60) + "</span></div>";
    html += "<form method=\"get\"><table cellspacing=\"5px\">";
    String nopt[] = {"Mode", "Colors", "Brightness", "Highlight"};
    int nnopt[] = {4, 3, 10, 2};
    for (int j = 0; j < sopts; j++) {
      html += "<tr><td>" + nopt[j] + "</td><td class=\"right\"><select name=\"" + j + "\" dir=\"rtl\">";
      String onoff[] = {"off", "on"};
      for (int i = 0; i < nnopt[j]; i++) {
        html += "<option value=\"" + String(i) + "\"";
        if (opt[j] == i) {
          html += "selected";
        }
        if (nnopt[j] == 2) {
          html += ">" + onoff[i] + "</option>";
        } else {
          html += ">" + String(i + 1) + "</option>";
        }
      }
      html += "</select></td></tr>";
    }
    html += "<tr><td>Distance</td><td><input name=\"" + String(sopts) + "\" value=\"" + String(opt[sopts]) + "\"></td></tr>";
    html += "<tr><td colspan=\"2\"><button type=\"submit\">Apply</button></td></tr>";
    html += "<tr><td colspan=\"2\"><button type=\"submit\" name=\"" + String(sopts + 1) + "\" value=\"1\">Save</button></td></tr>";
  } else {
    html += "<form style=\"margin-top: 5px\" method=\"get\"><table cellspacing=\"5px\">";
    html += "<tr><td>Battery</td><td class=\"right\">" + String(battery) + "%</td></tr>";
    html += "<tr><td>Uptime</td><td class=\"right\">" + String(millis() / 1000 % 86400 / 3600) + ":" + String(millis() / 1000 % 3600 / 60) + ":" + String(millis() / 1000 % 60) + "</td></tr>";
    html += "<tr><td>Distance</td><td class=\"right\">" + String(distance * radius / 1000) + "m</td></tr>";
    html += "<tr><td>Tgt Distance</td><td class=\"right\">" + String(opt[sopts]) + "m</td></tr>";
    html += "<tr><td>Total Distance</td><td class=\"right\">" + String(odistance + distance * radius / 1000) + "m</td></tr>";
    html += "<tr><td>Max Speed</td><td class=\"right\">" + String(maxkmh) + "km/h</td></tr>";
    html += "<tr><td>Avg Speed</td><td class=\"right\">" + String(distance * radius * 3.6 / millis()) + "km/h</td></tr>";
    html += "<tr><td colspan=\"2\"><button type=\"submit\" name=\"" + String(sopts + 2) + "\" value=\"0\">Return</button></td></tr>";
    html += "<tr><td colspan=\"2\"><button type=\"submit\" name=\"" + String(sopts + 1) + "\" value=\"2\">Export</button></td></tr>";
  }
  html += "<tr><td colspan=\"2\"><button type=\"submit\">Refresh</button></td></tr></table></form></body>";
  if (battery < 10) {
    html += "<script type=\"text/javascript\">alert(\"Battery at " + String(battery) + "%\");</script>";
  }
  server.send(200, "text/html", html);
}

void handleJSON() {
  String json;
  json += "{";
  json += "\n\t\"battery\":" + String(readBatteryPercentage());
  json += ",\n\t\"battery_raw\":" + String(analogRead(0));
  json += ",\n\t\"uptime\":" + String(millis() / 1000);
  json += ",\n\t\"distance\":" + String(distance * radius / 1000);
  json += ",\n\t\"total_distance\":" + String(odistance + distance * radius / 1000);
  json += ",\n\t\"saved_distance\":" + String(odistance * radius / 1000);
  json += ",\n\t\"target_distance\":" + String(opt[sopts]);
  json += ",\n\t\"max_speed\":" + String(maxkmh);
  json += ",\n\t\"avg_speed\":" + String(distance * radius * 3.6 / millis());
  json += ",\n\t\"mode\":" + String(opt[0]);
  json += ",\n\t\"color\":" + String(opt[1]);
  json += ",\n\t\"brightness\":" + String(opt[2]);
  json += ",\n\t\"hl_dot\":" + String(opt[3]);
  json += "\n}";
  server.send(200, "text/plain", json);
}

float readDistance() {
  long tmp = 0;
  for (int j = 0; j < dpos; j++) {
    tmp += EEPROM.read(j) * pow(255, j);
    //Serial.println(tmp);
  }
  return tmp;
}
void writeDistance(int tmp) {
  for (int j = 0; j < dpos; j++) {
    EEPROM.write(j, tmp % 255);
    //Serial.println(tmp % 255);
    tmp = tmp / 255;
  }
  EEPROM.commit();
}

int readBatteryPercentage() {
  int raw = analogRead(0);
  //int percentage = (raw - 243) * 1.5;
  int percentage = (raw - 275) * 0.87;
  return percentage;
}

/*
   opt0 = mode
   opt1 = color
   opt2 = brightness
   opt3 = hl dot
   opt[sopts] = target distance
   opt[sopts+1] = save
   opt[sopts+2] = html mode
   odistance = saved distance
*/
