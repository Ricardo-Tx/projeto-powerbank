#include <ESP8266WiFi.h>
#include "credentials.h"

// WIFI
const char* ssid = STASSID;
const char* password = STAPSK;

const char* server = "api.thingspeak.com";
char* request = "GET /update?api_key=" THINGSPEAK_TOKEN "&field1=3.67&field2=53.4&field3=3.00&field4=30.0 HTTP/1.1\r\nHost: api.thingspeak.com\r\nConnection: keep-alive\r\n\r\n";

WiFiClient client;
const int rate = 15000;

// BATTERY
const float drainResistance = 99.1;
const float voltageDivider = 1.240736553060895;

float voltage = 0.0;  // V
float current = 0.0;  // mA
float charge = 0.0;   // mAh
int id = 0;

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected\nIP address");
  Serial.println(WiFi.localIP());

  // Set time via NTP, as required for x.509 validation
  // configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  // Serial.print("Waiting for NTP time sync: ");
  // time_t now = time(nullptr);
  // while (now < 8 * 3600 * 2) {
  //   delay(500);
  //   Serial.print(".");
  //   now = time(nullptr);
  // }
  // Serial.println("");
  // struct tm timeinfo;
  // gmtime_r(&now, &timeinfo);
  // Serial.print("Current time: ");
  // Serial.print(asctime(&timeinfo));

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D0, INPUT);
}

long lastMsg = 0;
bool btnPressed = 0;
void loop() {
  long time = millis(); 
  if(time >= lastMsg + rate) {
    voltage = (analogRead(A0)+0.5) / 1024.0 * 3.3 * voltageDivider;
    current = voltage/drainResistance * 1000.0;
    charge += current * rate / 3600000.0; // ms to h
    
    Serial.print("U = ");
    Serial.print(voltage);
    Serial.println(" V");

    //SendRequest(1, voltage);
    SendRequest();
    lastMsg = time;
  }

  bool btnState = !digitalRead(D0);
  if(btnState && !btnPressed) {
    btnPressed = true; 
    id = (id+1)%20;
    Serial.print("ID is ");
    Serial.println(id);
  }
  if(!btnState && btnPressed) {
    btnPressed = false;
  }

  // digitalWrite(BUILTIN_LED, HIGH);
  // delay(1000);
  // digitalWrite(BUILTIN_LED, LOW);
  // delay(1000);
  //Serial.println(digitalRead(D0));
}

void SendRequest() {
  if (!client.connect(server, 80)) {
    Serial.println("Connection failed");
    return;
  }

  //*(request+42) = (char)(field+'0');

  // sprintf(request+44, "%.2f", value);
  *(request+44) = (char)((int)(voltage)%10+'0');
  *(request+46) = (char)((int)(voltage*10)%10+'0');
  *(request+47) = (char)((int)(voltage*100)%10+'0');

  *(request+56) = (char)((int)(current/10)%10+'0');
  *(request+57) = (char)((int)(current)%10+'0');
  *(request+59) = (char)((int)(current*10)%10+'0');

  // *(request+68) = (char)((int)(current/10)%10+'0');
  // *(request+70) = (char)((int)(current)%10+'0');
  // *(request+71) = (char)((int)(current*10)%10+'0');

  *(request+80) = (char)((int)(id/10)%10+'0');
  *(request+81) = (char)((int)(id)%10+'0');

  client.print(request);
  Serial.println(request);
  
  // delay(1000);
  // Serial.println(String(client.read()));
  //client.stop();
}
