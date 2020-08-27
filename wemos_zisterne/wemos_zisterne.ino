#include <ESP8266WiFi.h>

const char* ssid = "XXX";
const char* password = "XXX";

WiFiServer server(80);

int trigger = 5; // pin D1
int echo = 4; // pin D2

int max_hoehe = 20; // in cm
int max_tiefe = 147; // in cm
int grundflaeche = 41548; // in cm2

long dauer = 0;
long entfernung = 0; // in cm
int wasser_volumen = 0; // in cm3
float fuellstand_prozent = 0.0; // in prozent
int fuellstand_anzeige = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();

  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);

  // Connect to WiFi network

  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {

//messwert holen
  digitalWrite(trigger, LOW);
  delay(5);
  digitalWrite(trigger, HIGH);
  delay(10);
  digitalWrite(trigger, LOW);
  dauer = pulseIn(echo, HIGH);
  entfernung = (dauer / 2) / 29.1;

// header html
  WiFiClient client = server.available();
  client.println("HTTP/1.1 200 OK");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html xmlns='http://www.w3.org/1999/xhtml'>");
  client.println("<head><meta charset=\"UTF-8\">");
  client.println("<title>Wasserstand Regenwasserzisterne</title>");
  client.println("<meta http-equiv=\"refresh\" content=\"2\">"); //auto refresh alle 2 sek
  client.println("</head>\n<body>");
  client.println("<H2>Wasserstand Regenwasserzisterne</H2>");


  if (entfernung >= (max_tiefe + 30) || entfernung <= 0)
  {
    client.println("<h2>Fehler: Unrealistischer Messwert!</h2>");
    client.println("<h3>");
    client.println("Messung: ");
    client.print(entfernung);
    client.println(" cm <br>");
    client.println("</h3>");
  }
  else
  {
// anzeige: berechnung prozent und liter
    client.println("<h2>");
    fuellstand_prozent = 100.0 - ((100.0 / (max_tiefe-max_hoehe)) * (entfernung-max_hoehe));
    client.print(fuellstand_prozent);
    client.print(" % voll <br>");
    client.println("</h2>");
    client.println("<h3>");
    wasser_volumen = ((max_tiefe - entfernung) * grundflaeche) * 0.001;
    client.print(wasser_volumen);
    client.println(" Liter <br>");
    client.println("</h3>");
    
// grafik
    fuellstand_anzeige = (int)((300.0 * fuellstand_prozent) / 100.0);
    client.println("<svg width=\"400\" height=\"400\">");
    client.println("<rect x=\"100\"  width=\"150\" height=\"300\" style=\"fill:none;stroke:black;stroke-width:6;opacity:0.7\" />");
    client.println("<rect x=\"103\" y=\"");
    client.print(300 - fuellstand_anzeige); // von 0 bis 300
    client.println("\"  width=\"144\" height=\"");
    client.print(fuellstand_anzeige); // von 0 bis 300
    client.println("\" style=\"fill:blue;stroke:none;stroke-width:6;opacity:0.7\" />");
    client.println("</svg>");

// anzeige messung for debug
    client.println("<p>");
    client.println("Messung: ");
    client.print(entfernung);
    client.println(" cm <br>");
    client.println("</p>");
  }
  
  client.print("</body>\n</html>");
  delay(500);
}
