#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <DFRobot_ENS160.h>

Adafruit_AHTX0 aht;

#define I2C_COMMUNICATION  // Communication I2C. Commentez cette ligne si vous voulez utiliser la communication SPI.

#ifdef  I2C_COMMUNICATION
 DFRobot_ENS160_I2C ENS160(&Wire, /*I2CAddr*/ 0x53);
#else
  uint8_t csPin = D3;
  DFRobot_ENS160_SPI ENS160(&SPI, csPin);
#endif

long i = 0;


const char* ssid = "NomDuReseau";      // Nom du réseau Wi-Fi de l'AP
const char* password = "MotDePasse";   // Mot de passe du réseau Wi-Fi de l'AP
String txt [10]= {"TempL ","TempH ", "TempL2","TempH2","COL   ","COH   ","COL2  ","COH2  ", "HumL2  ","HumH2"};

const IPAddress ip(192, 168, 4, 1);
const IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

bool switchStates[5] = {false, false, false, false, false};
float textboxValues[5][2];  // Array to store values for 5 switches and 2 textboxes each

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
uint16_t varco2 = 0; // Variable à envoyer et afficher
double vartemp = 0; // Variable à envoyer et afficher
double varhum = 0; // Variable à envoyer et afficher

uint8_t Status =0;
uint8_t AQI =0; // Indice de qualité de l'air : 1-Excellent, 2-Bon, 3-Moderé, 4-Mauvais, 5-Malsain
uint16_t TVOC =0; // Concentration de composés organiques volatils totaux
uint16_t ECO2 =0; // CO2


void readSensor() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  
  varco2 = ECO2;
  vartemp = temp.temperature;
  varhum = humidity.relative_humidity;

  Status = ENS160.getENS160Status();
  AQI = ENS160.getAQI();
  TVOC = ENS160.getTVOC();
  ECO2 = ENS160.getECO2();

  varco2 = ECO2;    
  vartemp = temp.temperature;
  varhum = humidity.relative_humidity; 
  
  delay(200);  // Attendre 2 secondes entre chaque lecture
}

void updateSensorsData() {          //!!!!!!!!!!!===========<>>>>>>>>>
  readSensor();
  //////////////////////////////////////////
  String varco2Str = String(varco2);
  String vartempStr = String(vartemp);
  String varhumStr = String(varhum);

  updateVarco2Value();
  updateVartempValue();
  updateVarhumValue();
  
  //////////////////////////////////////////
}


void updateVarco2Value() {
  String script = "document.getElementById('varco2Value').innerHTML = ' CO2 Concentration : ' + " + String(varco2) + ";";
  server.send(200, "text/plain", script);
}

void updateVartempValue() {
  String script = "document.getElementById('vartempValue').innerHTML = ' Temperature : ' + " + String(vartemp) + ";";
  server.send(200, "text/plain", script);
}

void updateVarhumValue() {
  String script = "document.getElementById('varhumValue').innerHTML = ' Humidity : ' + " + String(varhum) + ";";
  server.send(200, "text/plain", script);
}



void handleRoot() {
  String html = "<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\"></head>";
  html += "<body style=\"display:flex; justify-content:center; align-items:center; height:100vh; background-color:white;\">";

  html += "<div style=\"text-align:center;\">";
  html += "<h1>Control Panel </h1>";
  html += "<H2> OV-PRO-TECH </h2>";

  html += "<p id=\"varco2Value\"> CO2 Concentration : <span>" + String(varco2) + "</span></p>";
  html += "<p id=\"vartempValue\">Temperature : <span>" + String(vartemp) + "</span></p>";
  html += "<p id=\"varhumValue\">Humidity : <span>" + String(varhum) + "</span></p>";


  for (int i = 0; i < 5; i++) {
    html += "<div style=\"display: flex; flex-direction: row;\">"; // Use flexbox to display in a row
    
    // Left textbox
    html += "<div style=\"margin-right: 10px;\">";
    html += txt[i * 2]  + ": <input type=\"text\" id=\"textbox" + String(i * 2) + "l\" class=\"textbox\"><br>";
    html += "</div>";
    
    // Switch
    html += "<div style=\"margin-down: 10px;\">";
    html += "<p> <label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleSwitch(" + String(i) + ")\" " + String(switchStates[i] ? "checked" : "") + "><span class=\"slider\"></span></label></p>";
    html += "</div>";
    
    // Right textbox
    html += "<div style=\"margin-right: 10px;\">";
    html += txt[i * 2 + 1] + ": <input type=\"text\" id=\"textbox" + String(i * 2 + 1) + "r\" class=\"textbox\"><br>";
    html += "</div>";
    
    html += "</div>";
  }

  // Submit button
  html += "<button onclick=\"submitData()\">Submit</button>";

  html += "<script>";
  html += "function toggleSwitch(switchNumber) {";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.open('GET', '/toggle?switch=' + switchNumber, true);";
  html += "xhttp.send();";
  html += "}";

/////////////////////////////////////////   //!!!!!!!!!!!===========<>>>>>>>>>
    
    html += "function updateValues() {";
    html += "  var xhttp = new XMLHttpRequest();";
    html += "  xhttp.onreadystatechange = function() {";
    html += "    if (this.readyState == 4 && this.status == 200) {";
    html += "      var values = JSON.parse(this.responseText);";
    html += "      document.getElementById('varco2Value').innerHTML = 'CO2 Concentration : <span>' + values.varco2 + '</span>';";
    html += "      document.getElementById('vartempValue').innerHTML = 'Temperature : <span>' + values.vartemp + '</span>';";
    html += "      document.getElementById('varhumValue').innerHTML = 'Humidity : <span>' + values.varhum + '</span>';";
    html += "    }";
    html += "  };";
    html += "  xhttp.open('GET', '/values', true);";
    html += "  xhttp.send();";
    html += "}";
      
    html += "setInterval(updateValues, 1000);"; // Update values every 1 second
    

////////////////////////////////////////

  html += "function submitData() {";
  for (int i = 0; i < 5; i++) {
    // Left textbox value
    html += "var textboxValue" + String(i * 2) + "l = parseFloat(document.getElementById('textbox" + String(i * 2) + "l').value);";

    // Right textbox value
    html += "var textboxValue" + String(i * 2 + 1) + "r = parseFloat(document.getElementById('textbox" + String(i * 2 + 1) + "r').value);";

    // Send both left and right textbox values
    html += "var xhttp" + String(i) + "l = new XMLHttpRequest();";
    html += "xhttp" + String(i) + "l.open('GET', '/submit?switch=" + String(i) + "&textboxValue" + String(i * 2) + "l=' + textboxValue" + String(i * 2) + "l, true);";
    html += "xhttp" + String(i) + "l.send();";

    html += "var xhttp" + String(i) + "r = new XMLHttpRequest();";
    html += "xhttp" + String(i) + "r.open('GET', '/submit?switch=" + String(i) + "&textboxValue" + String(i * 2 + 1) + "r=' + textboxValue" + String(i * 2 + 1) + "r, true);";
    html += "xhttp" + String(i) + "r.send();";
  }
  html += "}";
  html += "</script>";

//§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§§

  html += "<style>";
  
  html += ".textbox {";
  html += "    width: 150px; /* Changer la largeur selon vos besoins */";
  html += "    padding: 10px;";
  html += "    border: 2px solid #ccc;";
  html += "    border-radius: 10px;";
  html += "    margin-right: 2px;";
  html += "}";

  html += ".switch {";
  html += "position: relative;";
  html += "display: inline-block;";
  html += "width: 60px;";
  html += "height: 34px;";
  html += "}";

  html += ".slider {";
  html += "position: absolute;";
  html += "top: 0;";
  html += "left: 0;";
  html += "right: 0;";
  html += "bottom: 0;";
  html += "border-radius: 34px;";
  html += "background-color: #ccc;";
  html += "transition: .4s;";
  html += "}";

  html += ".slider:before {";
  html += "position: absolute;";
  html += "content: \"\";";
  html += "height: 26px;";
  html += "width: 26px;";
  html += "left: 4px;";
  html += "bottom: 4px;";
  html += "border-radius: 50%;";
  html += "background-color: white;";
  html += "transition: .4s;";
  html += "}";

  html += "input[type=\"checkbox\"] {";
  html += "display: none;";
  html += "}";

  html += "input[type=\"checkbox\"]:checked + .slider {";
  html += "background-color: #2196F3;";
  html += "}";

  html += "input[type=\"checkbox\"]:checked + .slider:before {";
  html += "transform: translateX(26px);";
  html += "}";

  html += ".switch-label {";
  html += "font-size: 12px;";
  html += "margin-top: 4px;";
  html += "}";

  html += "button {";
  html += "    padding: 16px 40px; /* Changer les valeurs de padding pour ajuster la taille */";
  html += "}";

  html += "</style>";
  
  html += "</div>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}


//:::::::::::::::::::::::::::::::::::::::::::::::::::
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
      String argNameLeft = "textboxValue" + String(switchNumber * 2) + "l";
      if (server.hasArg(argNameLeft)) {
        textboxValues[switchNumber][0] = server.arg(argNameLeft).toFloat();
      }

      // Right textbox value
      String argNameRight = "textboxValue" + String(switchNumber * 2 + 1) + "r";
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


void handleValues() {
  String jsonValues = "{\"varco2\": " + String(varco2) + ", \"vartemp\": " + String(vartemp) + ", \"varhum\": " + String(varhum) + "}";
  server.send(200, "application/json", jsonValues);
}

void setup() {
  while (!aht.begin() || NO_ERR != ENS160.begin()) {  
    Serial.println("Erreur de communication");
    delay(1000);
  }
  ENS160.setPWRMode(ENS160_STANDARD_MODE);
  ENS160.setTempAndHum(/*temperature=*/25.0, /*humidity=*/50.0);

  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(ip, ip, subnet);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggleSwitch);
  server.on("/submit", HTTP_GET, handleSubmit);
  server.on("/values", HTTP_GET, handleValues);
  
    
  server.begin();
  Wire.begin();
}



//ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù
//ùùùùùùùùùùùùùùùùùù  LOOP  ùùùùùùùùùùùùùùùùùùùùùùù
//ùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùùù

void loop() 
{  
  //sensors_event_t humidity, temp;
  //aht.getEvent(&humidity, &temp);
  
  


 updateSensorsData();  // Appel pour mettre à jour les données des capteurs

  
   Serial.print(varco2);
   Serial.print("   ");
   Serial.print(vartemp);
   Serial.print("    ");
   Serial.println(varhum);
   
   

  // Use the switch states and textbox values here in the loop
  for (int i = 0; i < 5; i++) {
    Serial.print("Switch " + String(i + 1) + " state: ");
    Serial.print(switchStates[i] ? "ON" : "OFF");

    // Left textbox value
    Serial.print("  Textbox " + String(i * 2 ) + " value: ");
    Serial.print(textboxValues[i][0]);

    // Right textbox value
    Serial.print("  Textbox " + String(i * 2 + 1) + " value: ");
    Serial.println(textboxValues[i][1]);
  } 
  
  server.handleClient();
}
