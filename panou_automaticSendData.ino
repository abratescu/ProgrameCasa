#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <avr/wdt.h>

#define DHTPIN 7
#define DHTTYPE DHT22
#define RELAYS 2
#define DEVICES 13
#define DEBUG
#define ONE_WIRE_BUS 8
#define TEMPERATURE_PRECISION 12

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress t0 = {0x28, 0x90, 0xBC, 0xFE, 0x05, 0x00, 0x00, 0x90  };//sufragerie centru
DeviceAddress t1 = {0x28, 0x14, 0x98, 0xFE, 0x05, 0x00, 0x00, 0x41  };//sufragerie stanga
DeviceAddress t2 = {0x28, 0x54, 0xA9, 0xFF, 0x05, 0x00, 0x00, 0xF2  };//dormitor (birou)
DeviceAddress t3 = {0x28, 0xCC, 0xAB, 0x34, 0x04, 0x00, 0x00, 0x67  };//Bucatarie 1
DeviceAddress t4 = {0x28, 0x5C, 0x00, 0xD0, 0x04, 0x00, 0x00, 0xD2  };//Sufragerie dreapta
DeviceAddress t5 = {0x28, 0x82, 0xC8, 0xFD, 0x05, 0x00, 0x00, 0x5F  };//Bucatarie 3
DeviceAddress t6 = {0x28, 0xEA, 0x0C, 0xD0, 0x04, 0x00, 0x00, 0x46  };//Bucatarie 2
DeviceAddress t7 = {0x28, 0x01, 0x65, 0xFD, 0x05, 0x00, 0x00, 0x33  };//Temperatura retur
DeviceAddress t8 = {0x28, 0x41, 0x57, 0xFD, 0x05, 0x00, 0x00, 0x41  };//Baie
DeviceAddress t9 = {0x28, 0x25, 0x49, 0x34, 0x04, 0x00, 0x00, 0x81  };//Temperatura tur
DeviceAddress t10 = {0x28, 0x35, 0xE1, 0xFE, 0x05, 0x00, 0x00, 0x5E  };//
DeviceAddress t11 = {0x28, 0x3B, 0x5D, 0xFE, 0x05, 0x00, 0x00, 0x23  };//Sufragerie fereastra
DeviceAddress t12 = {0x28, 0x47, 0xF3, 0xD0, 0x04, 0x00, 0x00, 0x99  };//Atelier beci

EthernetClient client;
byte mac[] ={0xAF, 0x42, 0xB8, 0x18, 0xE9, 0x0F};
IPAddress ip(192, 168, 2, 203);
short port=5000;
IPAddress server(192, 168, 2, 55);
String datReq;
EthernetUDP Udp;
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

bool relayState[RELAYS];
int pinRelay[RELAYS];
float dhtHumd;
float t[DEVICES+1];

long previousMillis = 0;        // will store last time values were read
long interval = 60000*7;           // interval at which to read (milliseconds)

void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  Ethernet.begin( mac, ip);
  Udp.begin(port);
  dht.begin();
  sensors.begin();
  wdt_enable(WDTO_8S);// setat la 8 secunde 
  
  delay(1500);
  while (!Serial);
  pinRelay[0]=5;pinRelay[1]=6;
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
  sensors.setResolution(t3, TEMPERATURE_PRECISION);
  sensors.setResolution(t4, TEMPERATURE_PRECISION);
  sensors.setResolution(t5, TEMPERATURE_PRECISION);
  sensors.setResolution(t6, TEMPERATURE_PRECISION);
  sensors.setResolution(t7, TEMPERATURE_PRECISION);
  sensors.setResolution(t8, TEMPERATURE_PRECISION);
  sensors.setResolution(t9, TEMPERATURE_PRECISION);
  sensors.setResolution(t10, TEMPERATURE_PRECISION);
  sensors.setResolution(t11, TEMPERATURE_PRECISION);
  sensors.setResolution(t12, TEMPERATURE_PRECISION);
}
void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
    readData();
    #ifdef DEBUG
    printData();
    #endif
    send_data();
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
    if(datReq.indexOf("releu")>=0)setRelays(datReq);
      else {
        #ifdef DEBUG
        Serial.println("This is not a valid command!");
        #endif
     }
  }
  if(packetSize)memset(packetBuffer, 0, UDP_TX_PACKET_MAX_SIZE);
  wdt_reset();
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
      client.print( "GET /add_data_panou.php?");
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
      client.print("&&");
      client.print("umid");
      client.print("=");
      client.print(dhtHumd);
      client.print("&&");
      client.print("a0");
      client.print("=");
      client.print(analogRead(A0));
      client.print("&&");
      client.print("a1");
      client.print("=");
      client.print(analogRead(A1));
      client.print("&&");
      client.print("a2");
      client.print("=");
      client.print(analogRead(A2));
      client.print("&&");
      client.print("a3");
      client.print("=");
      client.print(analogRead(A3));
      client.print("&&");
      client.print("a3");
      client.print("=");
      client.print(analogRead(A3));
      client.print("&&");
      double a = analogRead(A4)*0.02854-4.0976;
      int a4=(int)(a*1000);
      int rawValue=analogRead(A5);
      double b=0.04218*rawValue-25.65;
      if(b<0)b=0;
      int a5=(int)(b*1000);
      client.print("a4");
      client.print("=");
      client.print(a4);
      client.print("&&");
      client.print("a5");
      client.print("=");
      client.print(a5);
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
  t[3]=getTemp(t3);
  t[4]=getTemp(t4);
  t[5]=getTemp(t5);
  t[6]=getTemp(t6);
  t[7]=getTemp(t7);
  t[8]=getTemp(t8);
  t[9]=getTemp(t9);
  t[10]=getTemp(t10);
  t[11]=getTemp(t11);
  t[12]=getTemp(t12);
  t[DEVICES] = dht.readTemperature();
  dhtHumd  = dht.readHumidity();
}

#ifdef DEBUG
void printData(void){
  for(int i=0;i<=DEVICES;i++){
    Serial.print(t[i]);
    Serial.print(" ");
  }
  Serial.print("U:");
  Serial.print(dhtHumd);
  Serial.print(" ");
  for(int i=0;i<RELAYS;i++)
    if(relayState[i]==true)Serial.print("1");
    else Serial.print("0");
  Serial.print("    ");
  int a0=analogRead(A0);
  Serial.print(" A0=");
  Serial.print(a0);
  int a1=analogRead(A1);
  Serial.print(" A1=");
  Serial.print(a1);
  int a2=analogRead(A2);
  Serial.print(" A2=");
  Serial.print(a2);
  int a3=analogRead(A3);
  Serial.print(" A3=");
  Serial.print(a3);
  int a4=analogRead(A4);
  Serial.print(" A4=");
  Serial.print(a4);
  int a5=analogRead(A5);
  Serial.print(" A5=");
  Serial.print(a5);
  double a = analogRead(A4)*0.02854-4.0976;
  Serial.print(" A4=");
  Serial.print((int)(a*1000));
  int rawValue=analogRead(A5);
  double b=0.04218*rawValue-25.65;
  Serial.print(" A5=");
  if(b<0)b=0;
  Serial.println((int)(b*1000));
}
void printConfig(void){
  Serial.println("PLEASE CHECK!!!\nConfiguration(panou):");
  Serial.println("|Relays|Sensors|      IP::Port    |");
  Serial.print("   ");Serial.print(RELAYS);Serial.print("\t  ");Serial.print(DEVICES+1);Serial.print("     ");
  Serial.print(ip);Serial.print(":");Serial.println(port);
  Serial.print("RelayPins:");
  for(int i=0;i<RELAYS;i++){
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
