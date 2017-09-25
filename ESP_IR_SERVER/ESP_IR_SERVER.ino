#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
const char* ssid = "ssid"
const char* password = "password"
MDNSResponder mdns;

ESP8266WebServer server(80);

IRsend irsend(0);
IRrecv irrecv(2);

int opt[2];
int rcode, rcodec, rbits, rtime;
int scode, scodec, sbits;
int code, codec, bits;
decode_results  results;

void setup(void) {
  irsend.begin();
  irrecv.enableIRIn();

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (mdns.begin("remote", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.on("/json", handleJSON);

  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  if (irrecv.decode(&results)) {
    if (results.decode_type != UNKNOWN && results.value != 4294967295) {
      Serial.println(results.value);
      rcode = results.value;
      rcodec = results.decode_type;
      rbits = results.bits;
      rtime = millis();
    }
    irrecv.resume();
  }
  delay(100);
}

void handleSend() {
  for (int i = 0; i < server.args(); i++) {
    opt[server.argName(i).toInt()] = server.arg(i).toInt();
  }
  scodec = opt[0];
  scode = opt[1];
  sbits = opt[2];
  switch (scodec) {
    default:
    case UNKNOWN:      Serial.print("UNKNOWN");             break;
    case NEC:          irsend.sendNEC(scode, sbits);        break;
    case SONY:         irsend.sendSony(scode, sbits);       break; //not tested
    case RC5:          Serial.print("Not supported yet.");  break;
    case RC6:          Serial.print("Not supported yet.");  break;
    case DISH:         irsend.sendDISH(scode, sbits);       break; //not tested
    case SHARP:        irsend.sendSharpRaw(scode, sbits);   break; //not tested
    case JVC:          irsend.sendJVC(scode, sbits);        break; //not tested
    case SANYO:        Serial.print("Not supported yet.");  break;
    case MITSUBISHI:   Serial.print("Not supported yet.");  break;
    case SAMSUNG:      irsend.sendSAMSUNG(scode, sbits);    break; //not tested
    case LG:           irsend.sendLG(scode, sbits);         break; //not tested
    case WHYNTER:      irsend.sendWhynter(scode, sbits);    break; //not tested
    case AIWA_RC_T501: Serial.print("Not supported yet.");  break;
    case PANASONIC:    irsend.sendPanasonic(0x4004, scode); break;
  }
  sendResponse(1);
}

void handleRoot() {
  sendResponse(0);
}

void handleJSON() {
  String json;
  json += "{\n  \"codec\": ";
  json += rcodec;
  json += ",\n  \"code\": ";
  json += rcode;
  json += ",\n  \"bitlength\": ";
  json += rbits;
  json += ",\n  \"time\": ";
  json += (millis() - rtime) / 1000;
  json += "\n}";
  server.send(200, "text/plain", json);
}

void sendResponse(int sent) {
  String html;
  html += "<style>a {font-size: 4vh;color: #d9d9d9;text-decoration: none;}a:hover {color: #ffffff;}p {font-size: 2vh;color: #d9d9d9;font-family: monospace;}body {margin: 0px;background: #1c1e21;font-family: sans-serif;}.wrapper {background: #262c35;max-width: 512px;margin: auto;padding: 1%;}</style><body><div class=\"wrapper\">";
  if (sent == 0) {
    html += "<a>Recieved " + String((millis() - rtime) / 1000) + "s ago.</a>";
    code = rcode;
    codec = rcodec;
    bits = rbits;
  } else {
    html += "<a>Sent</a>";
    code = scode;
    codec = scodec;
    bits = sbits;
  }
  html += "<p>Code: " + String(code) + "<br>";
  html += "Codec: " + String(codec) + "<br>";
  html += "Bitlength: " + String(bits) + "<br></p>";
  if (sent == 0) {
    html += "<a href=\"send?0=" + String(codec) + "&1=" + String(code) + "&2=" + String(bits) + "\">Link</a><br>";
    html += "<a href=\"\">Refresh</a></div></body>";
  } else {
    html += "<a href=\"send?0=" + String(codec) + "&1=" + String(code) + "&2=" + String(bits) + "\">Resend</a><br>";
    html += "<a href=\"/\">Back</a>";
  }
  server.send(200, "text/html", html);
}
