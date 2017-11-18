#include <Ethernet.h>
#include <EthernetUdp.h>
#define RELAYS 8
#define DEBUG
#include <avr/wdt.h>
EthernetClient client;
byte mac[] ={0x90, 0xA2, 0xDA, 0x0f, 0x25, 0xE7};
IPAddress ip(192, 168, 2, 205);
short port=5000;
IPAddress server(192, 168, 2, 55);
String datReq;
EthernetUDP Udp;
char packetBuffer[15];
bool relayState[RELAYS];
int pinRelay[RELAYS];

void setup() {
  wdt_enable(WDTO_8S);// setat la 8 secunde
  #ifdef DEBUG
  Serial.begin(9600);
  #endif
  Ethernet.begin( mac, ip);
  Udp.begin(port);
  delay(1500);
  while (!Serial);
  pinRelay[0]=4;pinRelay[1]=5;pinRelay[2]=6;pinRelay[3]=7;
  pinRelay[4]=8;pinRelay[5]=9;pinRelay[6]=2;pinRelay[7]=3;
  for(int i=0;i<8;i++){
    pinMode(pinRelay[i], OUTPUT);
    digitalWrite(pinRelay[i], HIGH);
  }
  #ifdef DEBUG
  Serial.println("");
  Serial.println("Setup done.");
  printConfig();
  printData();
  #endif
}
void loop() {
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
    send_data();
  }
  memset(packetBuffer, 0, UDP_TX_PACKET_MAX_SIZE);
   wdt_reset();
}
void setRelays(String datReq){
  int i=5;
      while(i){
        if(i-5>7)break;//string prea mare, nu am atatea relee
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
      client.print( "GET /add_data_panou_sus_hol.php?");
      for(int i=0;i<RELAYS;i++){
        client.print("releu");
        client.print(i+1);
        client.print("=");
        if(relayState[i])client.print("1");
        if(i!=RELAYS-1)client.print("&&");
      }
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
#ifdef DEBUG
void printData(void){
  for(int i=0;i<RELAYS;i++)
    if(relayState[i]==true)Serial.print("1");
    else Serial.print("0");
  Serial.println("");
}
void printConfig(void){
  Serial.println("PLEASE CHECK!!!\nConfiguration:");
  Serial.println("|Relays|Sensors|      IP::Port    |");
  Serial.print("   ");Serial.print(RELAYS);Serial.print("\t  ");Serial.print(0);Serial.print("     ");
  Serial.print(ip);Serial.print(":");Serial.println(port);
  Serial.print("RelayPins:");
  for(int i=0;i<8;i++){
    Serial.print(" ");
    Serial.print(pinRelay[i]);
  }
  Serial.println("");
  Serial.print("oneWireBus:");
  Serial.println("-");
  Serial.print("tempPrecision:");
  Serial.println("-");
}
#endif
