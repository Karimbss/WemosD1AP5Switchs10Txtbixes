#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


const char* ssid = "NomDuReseau";      // Nom du réseau Wi-Fi de l'AP
const char* password = "MotDePasse";   // Mot de passe du réseau Wi-Fi de l'AP

const IPAddress ip(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);



ESP8266WebServer server(80);

bool switchStates[5] = {false, false, false, false, false};
float textboxValues[5][2];  // Array to store values for 5 switches and 2 textboxes each

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>Interface de commande</h1>";

  for (int i = 0; i < 5; i++) {
    // Switch
    html += "<p>Switch " + String(i + 1) + ": <label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleSwitch(" + String(i) + ")\" " + String(switchStates[i] ? "checked" : "") + "><span class=\"slider\"></span></label></p>";

    // Left textbox
    html += "Textbox " + String(i * 2 + 1) + ": <input type=\"text\" id=\"textbox" + String(i * 2 + 1) + "l\"><br>";

    // Right textbox
    html += "Textbox " + String(i * 2 + 2) + ": <input type=\"text\" id=\"textbox" + String(i * 2 + 2) + "r\"><br>";
  }

  // Submit button
  html += "<button onclick=\"submitData()\">Submit</button>";

  html += "<script>";
  html += "function toggleSwitch(switchNumber) {";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.open('GET', '/toggle?switch=' + switchNumber, true);";
  html += "xhttp.send();";
  html += "}";

  html += "function submitData() {";
  for (int i = 0; i < 5; i++) {
    // Left textbox value
    html += "var textboxValue" + String(i * 2 + 1) + "l = parseFloat(document.getElementById('textbox" + String(i * 2 + 1) + "l').value);";

    // Right textbox value
    html += "var textboxValue" + String(i * 2 + 2) + "r = parseFloat(document.getElementById('textbox" + String(i * 2 + 2) + "r').value);";

    // Send both left and right textbox values
    html += "var xhttp" + String(i) + "l = new XMLHttpRequest();";
    html += "xhttp" + String(i) + "l.open('GET', '/submit?switch=" + String(i) + "&textboxValue" + String(i * 2 + 1) + "l=' + textboxValue" + String(i * 2 + 1) + "l, true);";
    html += "xhttp" + String(i) + "l.send();";

    html += "var xhttp" + String(i) + "r = new XMLHttpRequest();";
    html += "xhttp" + String(i) + "r.open('GET', '/submit?switch=" + String(i) + "&textboxValue" + String(i * 2 + 2) + "r=' + textboxValue" + String(i * 2 + 2) + "r, true);";
    html += "xhttp" + String(i) + "r.send();";
  }
  html += "}";

  html += "</script>";

  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleToggleSwitch() {
  if (server.hasArg("switch")) {
    int switchNumber = server.arg("switch").toInt();
    if (switchNumber >= 0 && switchNumber < 5) {
      switchStates[switchNumber] = !switchStates[switchNumber];
      server.send(200, "text/plain", String(switchStates[switchNumber] ? "1" : "0"));
    } else {
      server.send(400, "text/plain", "Invalid switch number");
    }
  } else {
    server.send(400, "text/plain", "Switch parameter missing");
  }
}

void handleSubmit() {
  if (server.hasArg("switch")) {
    int switchNumber = server.arg("switch").toInt();
    if (switchNumber >= 0 && switchNumber < 5) {
      // Left textbox value
      String argNameLeft = "textboxValue" + String(switchNumber * 2 + 1) + "l";
      if (server.hasArg(argNameLeft)) {
        textboxValues[switchNumber][0] = server.arg(argNameLeft).toFloat();
      }

      // Right textbox value
      String argNameRight = "textboxValue" + String(switchNumber * 2 + 2) + "r";
      if (server.hasArg(argNameRight)) {
        textboxValues[switchNumber][1] = server.arg(argNameRight).toFloat();
      }

      server.send(200, "text/plain", "Data submitted.");
    } else {
      server.send(400, "text/plain", "Invalid switch number");
    }
  } else {
    server.send(400, "text/plain", "Switch parameter missing");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ip, ip, subnet);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggleSwitch);
  server.on("/submit", HTTP_GET, handleSubmit);

  server.begin();
}

void loop() {
  // Use the switch states and textbox values here in the loop
  for (int i = 0; i < 5; i++) {
    Serial.print("Switch " + String(i + 1) + " state: ");
    Serial.println(switchStates[i] ? "ON" : "OFF");

    // Left textbox value
    Serial.print("  Textbox " + String(i * 2 + 1) + " value: ");
    Serial.println(textboxValues[i][0]);

    // Right textbox value
    Serial.print("  Textbox " + String(i * 2 + 2) + " value: ");
    Serial.println(textboxValues[i][1]);
  }

  server.handleClient();
}

