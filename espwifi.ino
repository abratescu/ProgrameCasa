#include <ESP8266WiFi.h>
#include <DHT.h>
#include <WiFiClient.h>

#define DHTPINA D4
const char* ssid = "parter";
const char* pass = "setrometal";

const char* host = "192.168.2.55";
DHT dht(DHTPINA, DHT22);
float dhtUmid,dhtTemp;
 
void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, pass);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
  
  dht.begin();
}
void loop() {
  readData();
  Serial.print("Temp,umid:");
  Serial.print(dhtTemp);
  Serial.print(",");
  Serial.println(dhtUmid);
  send_data();
  Serial.println("Going to sleep.");
  ESP.deepSleep(60e6);
}
void readData(void){
  dhtTemp = dht.readTemperature();
  dhtUmid  = dht.readHumidity();
}
void send_data(){
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/add_data_espwifi.php?";
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + "umid=" +dhtUmid + "&&" + "temp=" +dhtTemp + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
}
