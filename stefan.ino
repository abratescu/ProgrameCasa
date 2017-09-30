#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 7
#define DHTTYPE DHT11
#define RELAYS 8
#define DEVICES 3
#define DEBUG
#define ONE_WIRE_BUS 6
#define TEMPERATURE_PRECISION 12

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress t0 = { 0x28, 0x6C, 0xE4, 0xFD, 0x05, 0x00, 0x00, 0xBC };// geam S
DeviceAddress t1 = { 0x28, 0x43, 0x54, 0xFE, 0x05, 0x00, 0x00, 0xE9 };//caldura S
DeviceAddress t2 = { 0x28, 0xB7, 0x58, 0xFE, 0x05, 0x00, 0x00, 0x66 };//camera S

EthernetClient client;
byte mac[] ={0xAF, 0x43, 0xC8, 0x29, 0xEC, 0x0E};
IPAddress ip(192, 168, 2, 201);
short port=5000;
IPAddress server(192, 168, 2, 55);
String datReq;
EthernetUDP Udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

bool relayState[RELAYS];
bool prezenta;
int pinRelay[RELAYS];
float dhtHumd;
float t[DEVICES+1];

long previousMillis = 0;        // will store last time values were read
long interval = 2000;           // interval at which to read (milliseconds)

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  Ethernet.begin( mac, ip);
  Udp.begin(port);
  dht.begin();
  sensors.begin();
  delay(1500);
  while (!Serial);
  pinRelay[0]=0;pinRelay[1]=7;pinRelay[2]=6;pinRelay[3]=1;
  pinRelay[4]=2;pinRelay[5]=7;pinRelay[6]=8;pinRelay[7]=9;
  for(int i=0;i<8;i++){
    pinMode(pinRelay[i], OUTPUT);
    digitalWrite(pinRelay[i], HIGH);
  }
  #ifdef DEBUG
  Serial.println("");
  Serial.println("Setup done.");
  printConfig();
  #endif
  readData();// read because of errors if request sent in the first interval
  #ifdef DEBUG
    printData();
  #endif
  sensors.setResolution(t0, TEMPERATURE_PRECISION);
  sensors.setResolution(t1, TEMPERATURE_PRECISION);
  sensors.setResolution(t2, TEMPERATURE_PRECISION);
}
void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    // save the last time
    previousMillis = currentMillis;
    readData();
    #ifdef DEBUG
    printData();
    #endif
   }
  int packetSize =Udp.parsePacket();
  if(packetSize) {
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    String datReq(packetBuffer);
    #ifdef DEBUG
    Serial.println(datReq);
    Serial.print("PacketSize=");
    Serial.println(packetSize);
    #endif
    if (datReq=="data")send_data();
    else if(datReq.indexOf("releu")>=0)setRelays(datReq);
    else {
      #ifdef DEBUG
      Serial.println("This is not a valid command!");
      #endif
    }
  }
  if(packetSize)memset(packetBuffer, 0, UDP_TX_PACKET_MAX_SIZE);
}
void setRelays(String datReq){
  int i=5;
      while(i){
        if(i-5>RELAYS-1)break;//string prea mare, nu am atatea relee
        if(datReq.charAt(i)!='0'&&datReq.charAt(i)!='1')break;
        if(datReq.charAt(i)=='1'){
          relayState[i-5]=true;
          digitalWrite(pinRelay[i-5], LOW);
        }
        else if(datReq.charAt(i)=='0'){
          relayState[i-5]=false;
          digitalWrite(pinRelay[i-5], HIGH);
        }
        i++;
      }
}
void send_data(){
  if(client.connect(server, 80)) {
      #ifdef DEBUG
      Serial.println("-> Connected");
      #endif
      // Make a HTTP request:
      client.print( "GET /add_data_stefan.php?");
      for(int i=0;i<=DEVICES;i++){
        client.print("temp");
        client.print(i+1);
        client.print("=");
        client.print(t[i]);
        client.print("&&");
      }
      for(int i=0;i<RELAYS;i++){
        client.print("releu");
        client.print(i+1);
        client.print("=");
        if(relayState[i])client.print("1");
        client.print("&&");
      }
      client.print("prezenta");
      client.print("=");
      if(prezenta)client.print("1");
      client.print("&&");
      client.print("&&");
      client.print("humid");
      client.print("=");
      client.print(dhtHumd);
      client.println( " HTTP/1.1");
      client.print( "Host: " );
      client.println(server);
      client.println( "Connection: close" );
      client.println();
      client.println();
      client.stop();
    }
    else {
      // you didn't get a connection to the server:
      #ifdef DEBUG
      Serial.println("--> connection failed/n");
      #endif
    }
}
float getTemp(DeviceAddress deviceAddress)
{
  return sensors.getTempC(deviceAddress);
}
void readData(void){
  sensors.requestTemperatures();
  delay(1000);
  t[0]=getTemp(t0);
  t[1]=getTemp(t1);
  t[2]=getTemp(t2);
  t[DEVICES] = dht.readTemperature();
  dhtHumd  = dht.readHumidity();
}
#ifdef DEBUG
void printData(void){
  for(int i=0;i<=DEVICES;i++){
    Serial.print(t[i]);
    Serial.print(" ");
  }
  Serial.print("  ");
  for(int i=0;i<RELAYS;i++)
    if(relayState[i]==true)Serial.print("1");
    else Serial.print("0");
  Serial.print(" p:");
  if(prezenta==true)Serial.print("1");
    else Serial.print("0");
  Serial.println("");
}
void printConfig(void){
  Serial.println("PLEASE CHECK!!!\nConfiguration(stefan):");
  Serial.println("|Relays|Sensors|      IP::Port    |");
  Serial.print("   ");Serial.print(RELAYS);Serial.print("\t  ");Serial.print(DEVICES+1);Serial.print("     ");
  Serial.print(ip);Serial.print(":");Serial.println(port);
  Serial.print("RelayPins:");
  for(int i=0;i<8;i++){
    Serial.print(" ");
    Serial.print(pinRelay[i]);
  }
  Serial.println("");
  Serial.print("oneWireBus:");
  Serial.println(ONE_WIRE_BUS);
  Serial.print("tempPrecision:");
  Serial.println(TEMPERATURE_PRECISION);
}
#endif
