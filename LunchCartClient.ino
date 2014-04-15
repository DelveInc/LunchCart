//////////////////////////////////////////////////////////////////
// Based off of ©2011 bildr
// Released under the MIT License - Please reuse change and share
// Modified to be used as a tracker and notification device for the
// Also uses the default WebClient Example provided with the 
// CC3000 library
//
// Design Concepts Lunch Cart
// created by Jack Boland
//////////////////////////////////////////////////////////////////

#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "Wifi-DCInternal"           // cannot be longer than 32 characters!
#define WLAN_PASS       ************		   // (Redacted)
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  20000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "www.google.com"
#define WEBPAGE      "/testwifi/index.html"

Adafruit_CC3000_Client www;
int count = 0;
boolean disconnected = false;

//Analog read pins
const int xPin = 0;
const int yPin = 1;
const int zPin = 2;

//to hold the calculated values
double x;
double y;
double z;

// initialize previous values
double prevX = 0;
double prevY = 0;
double prevZ = 0;

double deltaX;
double deltaY;
double deltaZ;

//The minimum and maximum values that came from
//the accelerometer while standing still
//You very well may need to change these
int minVal = 265;
int maxVal = 402;

uint32_t ip;
int startTime;

void setup(void)
{
  Serial.begin(115200);
  connectToServer();
  startTime = millis()/1000;
}

void loop(void)
{
 if(www.connected()){
   takeReading();  
   delay(1000);
 }
 else{
   Serial.println();
   Serial.println("Lost connection");
 }
 
}

/**************************************************************************/
/*!
    @brief  Begins an SSID scan and prints out all the visible networks
*/
/**************************************************************************/

void listSSIDResults(void)
{
  uint8_t valid, rssi, sec, index;
  char ssidname[33]; 

  index = cc3000.startSSIDscan();

  Serial.print(F("Networks found: ")); Serial.println(index);
  Serial.println(F("================================================"));

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);
    
    Serial.print(F("SSID Name    : ")); Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    // Serial.print(ipAddress, BIN);
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    
    Serial.println();
    //Serial.print(ipAddress, BIN);
    Serial.println();

  }
}

/**************************************************************************/
/*!
    @brief  Takes for numbers and turns them into a formatted IP Address
            -- Modified version of the CC3000 library funciton IP2U32
*/
/**************************************************************************/
uint32_t createIP(uint32_t first, uint32_t second, uint32_t third, uint32_t fourth)
{
    uint32_t ipAddress = first;          // Creates new value 
    ipAddress <<= 8;
    ipAddress |= second;
    ipAddress <<= 8;
    ipAddress |= third;
    ipAddress << 8;
    ipAddress |= fourth;
    
    return ipAddress;  
}

/**************************************************************************/
/*!
    @brief  Takes a reading from the accelerometer
*/
/**************************************************************************/
void takeReading()
{
    
  //read the analog values from the accelerometer
  int xRead = analogRead(xPin);
  int yRead = analogRead(yPin);
  int zRead = analogRead(zPin);

  //convert read values to degrees -90 to 90 - Needed for atan2
  int xAng = map(xRead, minVal, maxVal, -90, 90);
  int yAng = map(yRead, minVal, maxVal, -90, 90);
  int zAng = map(zRead, minVal, maxVal, -90, 90);

  //Calculate 360deg values like so: atan2(-yAng, -zAng)
  //atan2 outputs the value of -π to π (radians)
  //We are then converting the radians to degrees
  x = RAD_TO_DEG * (atan2(-yAng, -zAng) + PI);
  y = RAD_TO_DEG * (atan2(-xAng, -zAng) + PI);
  z = RAD_TO_DEG * (atan2(-yAng, -xAng) + PI);

  // Recalculate the delta values
  deltaX = x - prevX;
  deltaY = y - prevY;
  deltaZ = z - prevZ;
  
  // Make the current values the previous values
  prevX = x;
  prevY = y;
  prevZ = z;
 
  float total = sqrt(sq(deltaX) + sq(deltaY) + sq(deltaZ));
  Serial.println(total);
  if((int)total > 20){
      if(((millis()/1000) - startTime) > 60){
          Serial.println(startTime);
          startTime = millis()/1000;
          www.print("1");    
          Serial.print(1);
      }  
  }
  else{
      www.print("0"); 
      Serial.print(0);
  }
}

void connectToServer(){
  Serial.println(F("Hello, CC3000!\n")); 

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(3000);
  }

  
  ip = cc3000.IP2U32(***, **, **, **);			// IP Address (redacted)

  cc3000.printIPdotsRev(ip);
  Serial.println();
  Serial.println();
  www = cc3000.connectTCP(ip, 12345);
  
  
  if (www.connected()) {
    Serial.println("Connected");
    startTime = millis()/1000;
    Serial.println(startTime);
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("-------------------------------------"));

  Serial.println();
  Serial.println(F("-------------------------------------")); 
  
}
