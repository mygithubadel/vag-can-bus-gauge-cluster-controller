#include <stdio.h>
#include <stdint.h>
#include <SPI.h> 
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <LiquidCrystal.h>
#include <ESP8266TrueRandom.h>

struct UDPOutGaugePacket {
  uint32_t time;
  char car[4];
  uint16_t flags;
  uint8_t gear;
  uint8_t player_id;
  float speed;
  float rpm;
  float boost;
  float engtemp;
  float fuel;
  float oilpress;
  float oiltemp;
  uint32_t dashlights;
  uint32_t showlights;
  float throttle;
  float brake;
  float clutch;
  char display1[16];
  char display2[16];
  int32_t id;
};

#define LFS_MAX_PACKET_SIZE sizeof(UDPOutGaugePacket)

//DEBUG VARIABLES
bool serialEnabled = true;
bool simenabled = false;


// Set WiFi credentials
#define WIFI_SSID "----------"
#define WIFI_PASS "----------"
#define UDP_PORT 4444
IPAddress staticIP(10, 0, 0, 88);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(10, 0, 0, 1);
uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];

// UDP
WiFiUDP Udp;
char packet[255];
char reply[] = "Packet received!";

// CAN
#define CAN_2515
const int SPI_CS_PIN = 16;
const int CAN_INT_PIN = 2;
#include "mcp2515_can.h"
mcp2515_can CAN(SPI_CS_PIN); // Set CS pin

// TIMING
unsigned long currentMillis;
const int delayTime = 1;
static unsigned long previousMillis = 0;
int abstimer = 0;
int distancetimer = 0;
int timeAdder = 0;


////////////////////////

void CanSend(short address, byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h) {
  unsigned char DataToSend[8] = {a,b,c,d,e,f,g,h};
  CAN.sendMsgBuf(address, 0, 8, DataToSend);
}

//(Speed and RPM)
// SPEED CALCULATION FUNCTIONS
#define lo8(x) ((int)(x)&0xff)
#define hi8(x) ((int)(x)>>8)

int speed = 0;
byte speedL = 0;
byte speedH = 0;

short tempRPM = 0;
byte rpmL = 0;
byte rpmH = 0;

///////////////////////////////////////////////////////////////////////////////////////////////

int oiltemp = 90;
float mainspeed = 0;
int mainrpm = 0;
///////////////////////////////////////////////////////////////////////////////////////////////

//OG DATA

bool OGRightSignal = true;
bool OGLeftSignal = true;
bool OGABS = true;
bool OGHighBeam = false;
bool OGHandBrake = false;
bool OGFLDOOR = false;
bool OGFRDOOR = false;
bool OGRRDOOR = false;
bool OGRLDOOR = false;
bool OGHOOD = false;
bool OGKR = false;
///////////////////////////////////////////////////////////////////////////////////////////////

float ms = 0;

void updateDisplay(UDPOutGaugePacket * p)
{

  if(simenabled) simenabled = false;
  
  mainspeed = (float)(p->speed * 3.6);
  //Serial.println(mainspeed);
  uint16_t rpm = (uint16_t) p->rpm;

  mainrpm = (int)(((float)rpm/8000.0) * 6000.0);
  oiltemp = p->oiltemp;
  OGRightSignal = p->showlights & 0b1000000;
  OGLeftSignal = p->showlights &  0b0100000;
  OGABS = p->showlights & 0b10000000000;
  OGFLDOOR = p->showlights & 0b100000000000;
  OGFRDOOR = p->showlights & 0b1000000000000;
  OGRLDOOR = p->showlights & 0b10000000000000;
  OGRRDOOR = p->showlights & 0b100000000000000;
  OGHOOD = p->showlights & 0b1000000000000000;
  OGKR = p->showlights &   0b10000000000000000;
  
  OGHighBeam = p->showlights & 0b10;
  OGHandBrake = p->showlights & 0b100;
  setFuel((int)(p->fuel * 100.0));

}

int temp_turning_lights = 0;
int blinkvalue = 0;
int blink(char direction) {
  // 10  both
  // 00 left 
  // 01 right

  if(direction == 'z') return 0b100;

  //if(temp_turning_lights == 0b100){
    switch(direction){
      case 'r':
        temp_turning_lights = 0b110;
      break;
      case 'l':
        temp_turning_lights = 0b001;
      break;
        case 'b':
      temp_turning_lights = 0b111;
    }
    
  return temp_turning_lights;  
}

int getOilTemp(int c_oiltemp) {
  if (c_oiltemp>130) c_oiltemp = 130;
  if (c_oiltemp<50) c_oiltemp = 50;
  int out = 0;
  if(c_oiltemp >=50 && c_oiltemp <=90) {
    out = c_oiltemp ;
    out = out - 50;
    out = out + 130;
  }
  
  if(c_oiltemp >=90 && c_oiltemp <=130) {
    out = c_oiltemp ;
    out = out - 90;
    out = out + 200;
  }
  
  // 130 170 | 207 240
  return out;
}

int diff1;
int diff2;

void setFuel(int percentage) {
    if(percentage >= 0 && percentage <= 100) {
      diff1 = (int)((((float)percentage / 100.0) * 74.0) + 83.0);
      diff2 = (int)(((((float)percentage / 100.0) * 74.0) - 157.0) * -1.0);
  
      analogWrite(D2, diff1);
      analogWrite(D3, diff2);
    }
}


void setup()
{
   pinMode(5, OUTPUT); // handbrake
   pinMode(2, OUTPUT); // oil pressre
   digitalWrite(5, 1); // handbrake
   digitalWrite(2, 0); // oil pressure
   
   setFuel(50);

  if(serialEnabled) Serial.begin(115200);
  SPI.begin();
  if(serialEnabled){
    while (!Serial) {
      ;
    }
  }
  if(serialEnabled) Serial.println("outgague+can");

  if (WiFi.config(staticIP, gateway, subnet, dns, dns) == false) {
    if(serialEnabled) Serial.println("Configuration failed.");
  }
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  if(serialEnabled) Serial.print("Connecting to ");
  if(serialEnabled) Serial.print(WIFI_SSID);

  
  if(serialEnabled) Serial.println();
  if(serialEnabled) Serial.print("Connected! IP address: ");
  if(serialEnabled) Serial.println(WiFi.localIP());


  START_INIT:

  if (CAN_OK == CAN.begin(CAN_500KBPS)) 
  {
    if(serialEnabled) Serial.println("CAN BUS Shield init ok!");
  }
  else {
    if(serialEnabled) Serial.println("CAN BUS Shield init fail");
    if(serialEnabled) Serial.println("Init CAN BUS Shield again");
    delay(500);
    goto START_INIT;
  }

  delay(1000);

  Udp.begin(UDP_PORT);
  if(serialEnabled) Serial.println("udp.begin()");
}

bool wifiInit = false;
int distanceByteBeta = 0;
float distance50m = 0;

void loop()
{

  currentMillis = millis();

  if (currentMillis - previousMillis >= delayTime) {
    previousMillis = currentMillis;

    uint8_t remoteIp[4];
    uint16_t remotePort;
    
    if(WiFi.status() == WL_CONNECTED && !wifiInit) {
      OGLeftSignal = false;
      OGRightSignal = false;
      wifiInit = true;
    }
    
    if(wifiInit) {
      int16_t packetSize = Udp.parsePacket();
      if (packetSize) {
        int len = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
  
        String received_string(packetBuffer);
  
        if(serialEnabled) Serial.print(received_string);
        
        UDPOutGaugePacket *p =
            ((UDPOutGaugePacket *) & packetBuffer);
    
        updateDisplay(p);
      }
    }
      
    if(mainrpm > 200) digitalWrite(2, 1); else digitalWrite(2, 0); // oil pressure

    digitalWrite(5, !OGHandBrake);

    speed = ((mainspeed <= 270 ? mainspeed : 270) / 0.0075) + (ESP8266TrueRandom.random(1,50)); //KMH=1.12 MPH=0.62
    speedL = lo8(speed);
    speedH = hi8(speed);

    tempRPM = (mainrpm) * 4;
    rpmL = lo8(tempRPM);
    rpmH = hi8(tempRPM);

    if (timeAdder % 10 == 0) {
      
      //Immobilizer
      if(1 || mainrpm > 300)
        CanSend(0x3D0, 0, 0x80, 0, 0, 0, 0, 0, 0);

      //RPM
      if(1 || mainrpm > 300)
        CanSend(0x280, 0x49, 0x0E, rpmL, rpmH, 0x0E, 0x00, 0x1B, 0x0E);

      //Airbag
      CanSend(0x050, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    }

    if (timeAdder % 20 == 0 && simenabled) {
         mainrpm+=15;
         mainspeed += 0.35;
         if(mainspeed >= 270) {mainspeed = 0; mainrpm = 1000;}
         if (mainrpm > 6000) mainrpm = 4000;
    }


    abstimer++;

    if (abstimer == 10) {
      distance50m += (mainspeed / 3.6)/100.0;

      // for distance calculation, still dont know the exact expected sequence that is required for this byte, currently im using random values
      if(distance50m >= 1) {
        distanceByteBeta =  ESP8266TrueRandom.random(0,255);
        distance50m = distance50m - 1;
      }

      CanSend(0xDA0, 0, speedL, speedH, 0, 0, distanceByteBeta, 8, 0);
      CanSend(0x1A0, 0, (OGABS || mainrpm < 300 && 0 ? 0b10100001 : 0b10100000), 0, 0, 0, 0, 0, 0);

      abstimer = 0;

    }

    if(timeAdder % 50 == 0){
      
      //backlight
      CanSend(0x635, 100, 0, 0, 0, 0, 0x0, 0x0, 0x0);
      
      //oil
      CanSend(0x288,  ESP8266TrueRandom.random(0,255), getOilTemp(oiltemp),  ESP8266TrueRandom.random(0,255),  ESP8266TrueRandom.random(0,255),  ESP8266TrueRandom.random(0,255),  ESP8266TrueRandom.random(0,255),  ESP8266TrueRandom.random(0,255),  ESP8266TrueRandom.random(0,255));
      
      int doors = 0;
      if(OGRRDOOR) doors += 0b0100;
      if(OGRLDOOR) doors += 0b1000;
      if(OGFRDOOR) doors += 0b0010;
      if(OGFLDOOR) doors += 0b0001;
      if(OGHOOD) doors += 0b10000;
      if(OGKR) doors +=   0b100000;
      
      CanSend(0x470, 0, doors, 0, 0, 0, 0, 0, 0);

    }

    // BLINKING SIGNAL
    if(timeAdder % 10 == 0) {
      char blinkchar;
      if(OGRightSignal && OGLeftSignal) blinkchar = 'b';
      else if(OGRightSignal) blinkchar = 'r';
      else if(OGLeftSignal) blinkchar = 'l';
      else blinkchar = 'z';
  
      blinkvalue = blink(blinkchar);

      CanSend(0x531, (OGHighBeam ? 0xf : 0), 0, blinkvalue, 0, 0, 0, 0, 0);
    }

    if (timeAdder == 100000) timeAdder = 0;
    timeAdder++;

  }

}
