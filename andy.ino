#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Servo.h>

#define DHTPIN 9
#define DHTTYPE DHT22
#define RELAYS 4
#define DEVICES 6
//#define DEBUG
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 12
#define SERVOAPIN 7
#define SERVOSPIN 8

Servo myservoA; // 0-deschis 180-inchis !!
Servo myservoS;

DHT dht(DHTPIN, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress t0 = { 0x28, 0xCA, 0x1F, 0xFF, 0x05, 0x00, 0x00, 0x83 };// podea A
DeviceAddress t1 = { 0X28, 0X0F, 0X4A, 0X34, 0X04, 0X00, 0X00, 0XB6 };//camera A
DeviceAddress t2 = { 0x28, 0x4F, 0xB2, 0xFF, 0x05, 0x00, 0x00, 0x5D };// geam A
DeviceAddress t3 = { 0x28, 0xBF, 0x3F, 0xFF, 0x05, 0x00, 0x00, 0x08 };//caldura A
DeviceAddress t4 = { 0x28, 0x65 ,0x13 ,0xD0 ,0x4 , 0x0 , 0x0 , 0x9B };//pod
DeviceAddress t5 = { 0x28 ,0x3B ,0x40 ,0xFE ,0x5 , 0x0 , 0x0 , 0xB3 };//baie

EthernetClient client;
byte mac[] ={0xAF, 0x42, 0xB8, 0x28, 0xEC, 0x0E};
IPAddress ip(192, 168, 2, 202);
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
long interval = 25000;           // interval at which to read (milliseconds)

int servoAAngle=0;
int servoSAngle=0;
void setup() {
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  Ethernet.begin( mac, ip);
  Udp.begin(port);
  dht.begin();
  sensors.begin();
  myservoA.attach(SERVOAPIN);
  myservoS.attach(SERVOSPIN);
  
  delay(1500);
  while (!Serial);
  pinRelay[0]=3;pinRelay[1]=4;pinRelay[2]=5;pinRelay[3]=6;
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

  myservoA.write(servoAAngle);
  myservoS.write(servoSAngle);
  delay(250);
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
    else if(datReq.indexOf("servoa") != -1){
      int i=6;
      char c=datReq.charAt(i);
      int aux=0;
      while(c!='\0'){
        if(isDigit(c)){
          int r = c - '0';
          aux=aux*10+r;
        }
        else {
          #ifdef DEBUG
          Serial.println("This is not a valid command!");
          #endif
          return;
        }
        i++;
        c=datReq.charAt(i);
      }
      #ifdef DEBUG
      Serial.print("ServoA set at: ");
      Serial.println(aux);
      #endif
      myservoA.write(aux);
      servoAAngle=aux;
    }else if(datReq.indexOf("servos") != -1){
      int i=6;
      char c=datReq.charAt(i);
      int aux=0;
      while(c!='\0'){
        if(isDigit(c)){
          int r = c - '0';
          aux=aux*10+r;
        }
        else {
          #ifdef DEBUG
          Serial.println("This is not a valid command!");
          #endif
          return;
        }
        i++;
        c=datReq.charAt(i);
      }
      #ifdef DEBUG
      Serial.print("ServoS set at: ");
      Serial.println(aux);
      #endif
      myservoS.write(aux);
      servoSAngle=aux;
    }else {
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
      client.print( "GET /add_data_andy.php?");
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
      client.print("umid");
      client.print("=");
      client.print(dhtHumd);
      client.print("&&");
      client.print("servoa");
      client.print("=");
      client.print(servoAAngle);
      client.print("&&");
      client.print("servos");
      client.print("=");
      client.print(servoSAngle);
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
  Serial.println("PLEASE CHECK!!!\nConfiguration(andy):");
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
