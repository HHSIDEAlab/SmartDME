/*
 * This sketch implements the SDME Client for the ASPR / FEMA medical 
 * devicve energy proof of concept.
  *
 * This sketch is released to the public domain.
 *
 */

#include <WiFlyHQ.h>   //wiFly
#include <TinyGPS.h>   //GPS
#include <SoftwareSerial.h>
#include <string.h>

/* initialize objects */
TinyGPS gps;
WiFly wifly;
/* SD CARD Pinouts for Arduino
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4

/* Setup serial connections */
SoftwareSerial wifiSerial(9, 10);
SoftwareSerial gpsSerial(6, 4);
//SoftwareSerial goSerial(2, 3);
//SoftwareSerial gprsSerial(7, 8);


/* WiFi network settings*/
const char lzSSID[] = "smartDME";
const char lzPassword[] = "Smart-DME";

/* the site location and port */
const char lzSite[] = "ita2.seas.ucla.edu";
const int intPort = 80;
const char lzRequest[] = "GET /~fsanborn/firstresponder/index.php/testing/add/HASH1234/ID01/1/2013-09-09T14:31:03/BLOB/1/BatID/100/104/00.00:00/5/6/100/200/300/90/80/BATBITS/1/47.700614/-122.35021/1005/2012-10-29T14:31:03/10 HTTP/1.1";
char wiFlyBuf[32];

/* GPS */
unsigned long chars = 0;
unsigned short sentences = 0, failed = 0;
int gpsStatus = 0;
float gpsLongitude;                             // Longitude of the device
float gpsLatitude;                              // Latitude of the device
unsigned long gpsFixAge;                        // What is the fixed age
int gpsYear;
byte gpsMonth, gpsDay, gpsHour, gpsMinute, gpsSecond, gpsHundredths;
float gpsAltitude;
unsigned long gpsTime;                      // The timestamp from the GPS stream
unsigned long gpsDate;                    // The date fromt he gps stream
float gpsAccuracy;
int gpsDataType = 3;
int gpsUnit = 0 ;  // 0 = meteric  // 1 = standard (US)
// 
  static const float seaLat = 47.76173;
  static const float seaLong = -122.33554182;
  static const float seaAlt = 126.40;

void terminal();

void setup()
{

    /* set serial */
    Serial.begin(115200);
    Serial.println("Starting.......");
    Serial.println("---------------------------------------------");
    
    /* Set Wifly */
    Serial.print("Free memory: ");
    Serial.println(wifly.getFreeMemory(),DEC);
    wifiSerial.begin(9600);
    if (!wifly.begin(&wifiSerial, &Serial)) {
        Serial.println("Failed to start wifly");
	terminal();
    }

    /* Join wifi network if not already associated */
    if (!wifly.isAssociated()) {
	/* Setup the WiFly to connect to a wifi network */
	Serial.println("Joining network");
	wifly.setSSID(lzSSID);
	wifly.setPassphrase(lzPassword);
	wifly.enableDHCP();

	if (wifly.join()) {
	    Serial.println("Joined wifi network");
	} 
        else {
	    Serial.println("Failed to join wifi network");
	    terminal();
	}
    } 
    else {
        Serial.println("Already joined network");
    }
    
    /* Set the deviceID */
    wifly.setDeviceID("SmartDME-Client");
    
    /* Show Network Setting */
    Serial.print("MAC: ");
    Serial.println(wifly.getMAC(wiFlyBuf, sizeof(wiFlyBuf)));
    Serial.print("IP: ");
    Serial.println(wifly.getIP(wiFlyBuf, sizeof(wiFlyBuf)));
    Serial.print("Netmask: ");
    Serial.println(wifly.getNetmask(wiFlyBuf, sizeof(wiFlyBuf)));
    Serial.print("Gateway: ");
    Serial.println(wifly.getGateway(wiFlyBuf, sizeof(wiFlyBuf)));
    Serial.print("DeviceID: ");
    Serial.println(wifly.getDeviceID(wiFlyBuf, sizeof(wiFlyBuf)));

    /* See if the device is connected */
    if (wifly.isConnected()) {
        Serial.println("Old connection active. Closing");
	wifly.close();
    }
    
    /* Open the connection to the server */
    if (wifly.open(lzSite, intPort)) {
        Serial.print("Connected to ");
	Serial.println(lzSite);
                
        //fixed Data for testing
        //char* lztmpDeviceHash;
        //strcopy (lztmpDeviceHash, wifly.getMAC(wiFlyBuf, sizeof(wiFlyBuf)));
        //char* lztmpDeviceHash = wifly.getMAC(wiFlyBuf, sizeof(wiFlyBuf));  // Unique identifer (mac address for now)
        //lztmpDeviceHash = stripSemi(lztmpDeviceHash); 
        //long  deviceHash = hexToDec(lztmpDeviceHash);
        //char* deviceHash = (char) tmpDeviceHash;
        char* deviceHash = "0006668";                  
        //Serial.println(deviceHash);:
        char* deviceID = "SimplyGo0011";
        char* deviceFirmwareVersion = "1-1";
        int deviceDataType = 0;
        char* devicePowerStatus = "1";                   // Power Status from POC
        char* deviceDateTime = "2013-12-2T14:31:03";    // Internal Device Date and Time
        char* deviceBlob = "DeviceBlob";                 // Other device information to be used in the future
        char* deviceHasBattery = "1";                    // Does the device have a battery
        
        // Battery
        char* batteryID ="BATID";                        // Battery ID from POC
        char* batteryPowerLevel = "80";                  // Powerlevel of the Battery (0-100)
        char* batteryTemp = "60";                        // Time to empty (need to determine time unit)
        char* batteryAvgTimeToEmpty = "00";        // Average time to empty (how is this calculated)
        char* batteryAvgCurrent = "5";                   // Average Current (over what time)
        char* batteryAvgVoltage = "6";                   // Average voltage (over what time)
        char* batteryCycleCount = "1000";                // The number of battery cycles (from when?)
        char* batteryAccessCount = "2000";               // The number of time the battery has been read (from when?)
        char* batteryReadCount = "3000";                 // The number of read counts (need clarification)
        char* batteryIdealCapacity = "90";               // What is the ideal capacity of the battery (how is this expressed)
        char* batteryActualCapacity = "80";              // What is the actaul capacity of the battery (is this powerlevel?)
        char* batteryBits = "batterybits";               // Other battery information from the POC
        
        bool rReturnGPS = populateGPS();
               
        // Get the device time stamp
        // fetch the time
        //now = RTC.now();
        //char* lzDeviceTimeStamp = now.year() + "/";
        /* log time
        logfile.print(now.get()); // seconds since 2000
        logfile.print(", ");
        logfile.print(now.year(), DEC);
        logfile.print("/");
        logfile.print(now.month(), DEC);
        logfile.print("/");
        logfile.print(now.day(), DEC);
        logfile.print(" ");
        logfile.print(now.hour(), DEC);
        logfile.print(":");
        logfile.print(now.minute(), DEC);
        logfile.print(":");
        logfile.print(now.second(), DEC)*/
          
        // set the content type
        static char* lzContentType = "application/html";
        static char* lzAcceptLang = "en-us,en";
        static char* lzAccept = "text/html,application/xhtml+xml,application/xml;";
        static char* lzPostType = "GET /";
        static char* lzServerPath = "~fsanborn/firstresponder/index.php/";
        static char* lzParam = "testing/add/";
        
        // Device Information
        Serial.print(lzPostType);
        Serial.print(lzServerPath);
        Serial.print(lzParam);        
        Serial.print(deviceHash);                // Unique identifer (mac address for now)
        Serial.print("/");
        Serial.print(deviceID);                 // Device ID
        Serial.print("/");
        Serial.print(deviceDataType);                 // Device Data 0=Data from Device, cached data 2= test data, 
        Serial.print("/");
        Serial.print(deviceFirmwareVersion);
        Serial.print("/");

        Serial.print(devicePowerStatus);         // Power Status from POC
        Serial.print("/");
        Serial.print(deviceDateTime);            // Internal Device Date and Time
        Serial.print("/");
        Serial.print(deviceBlob);                // Other device information to be used in the future
        Serial.print("/");
        Serial.print(deviceHasBattery);          // Does the device have a battery
        Serial.print("/");
        
        // Battery
        Serial.print(batteryID);                 // Battery ID from POC
        Serial.print("/");
        Serial.print(batteryPowerLevel);         // Powerlevel of the Battery (0-100)
        Serial.print("/");
        Serial.print(batteryTemp);               // Time to empty (need to determine time unit)
        Serial.print("/");
        Serial.print(batteryAvgTimeToEmpty);     // Average time to empty (how is this calculated)
        Serial.print("/");
        Serial.print(batteryAvgCurrent);         // Average Current (over what time)
        Serial.print("/");
        Serial.print(batteryAvgVoltage);         // Average voltage (over what time)
        Serial.print("/");
        Serial.print(batteryCycleCount);         // The number of battery cycles (from when?)
        Serial.print("/");
        Serial.print(batteryAccessCount);        // The number of time the battery has been read (from when?)
        Serial.print("/");
        Serial.print(batteryReadCount);          // The number of read counts (need clarification)
        Serial.print("/");
        Serial.print(batteryIdealCapacity);      // What is the ideal capacity of the battery (how is this expressed)
        Serial.print("/");
        Serial.print(batteryActualCapacity);     // What is the actaul capacity of the battery (is this powerlevel?)
        Serial.print("/");
        Serial.print(batteryBits);               // Other battery information from the POC
        Serial.print("/");
                                              
        // GPS  
        Serial.print(gpsDataType);                 // GPS Data 0= current data from gps, 1= cached data, 2= test data        
        Serial.print("/");
        Serial.print(gpsStatus);                 // What is the status of the GPS unit (offline, online, other)
        Serial.print("/");
        Serial.print(gpsLongitude);              // Longitude of the device
        Serial.print("/");
        Serial.print(gpsLatitude);               // Latitude of the device
        Serial.print("/");
        Serial.print(gpsAltitude);               // Altitude of the device
        Serial.print("/");
        Serial.print(gpsTime);                   // The timestamp from the GPS stream
        Serial.print("/");
        Serial.print(gpsDate);                   // The date from the GPS Stream
        Serial.print("/");
        Serial.print(gpsFixAge);                 // What is the fixed age
        Serial.print("/");
        Serial.print(gpsAccuracy);               // The accuracy of the gps fix
        Serial.println(" HTTP/1.0");
        
        // Other Parameters 
        Serial.print("Host: ");
        Serial.println(lzSite);
        Serial.println("User-Agent: SMART-DME/0.0.1");
        Serial.print("Accept: ");
        Serial.println(lzAccept);
        Serial.print("Accept-Language: ");
        Serial.println(lzAcceptLang);
        Serial.println("Connection: keep-alive");
        Serial.println("Cache-Control: max-age=0");
        Serial.println("--------------------------------------");

//send to wifly
        wifly.print(lzPostType);
        wifly.print(lzServerPath);
        wifly.print(lzParam);        
        wifly.print(deviceHash);                // Unique identifer (mac address for now)
        wifly.print("/");
        wifly.print(deviceID);                 // Device ID from POC
        wifly.print("/");
        wifly.print(deviceFirmwareVersion);    // device firware version
        wifly.print("/");
        wifly.print(deviceDataType);                 // Device Data 0=Data from Device, cached data 2= test data,                 
        wifly.print("/");
        wifly.print(devicePowerStatus);         // Power Status from POC
        wifly.print("/");
        wifly.print(deviceDateTime);            // Internal Device Date and Time
        wifly.print("/");
        wifly.print(deviceBlob);                // Other device information to be used in the future
        wifly.print("/");
        wifly.print(deviceHasBattery);          // Does the device have a battery
        wifly.print("/");
        
        // Battery
        wifly.print(batteryID);                 // Battery ID from POC
        wifly.print("/");
        wifly.print(batteryPowerLevel);         // Powerlevel of the Battery (0-100)
        wifly.print("/");
        wifly.print(batteryTemp);               // Time to empty (need to determine time unit)
        wifly.print("/");
        wifly.print(batteryAvgTimeToEmpty);     // Average time to empty (how is this calculated)
        wifly.print("/");
        wifly.print(batteryAvgCurrent);         // Average Current (over what time)
        wifly.print("/");
        wifly.print(batteryAvgVoltage);         // Average voltage (over what time)
        wifly.print("/");
        wifly.print(batteryCycleCount);         // The number of battery cycles (from when?)
        wifly.print("/");
        wifly.print(batteryAccessCount);        // The number of time the battery has been read (from when?)
        wifly.print("/");
        wifly.print(batteryReadCount);          // The number of read counts (need clarification)
        wifly.print("/");
        wifly.print(batteryIdealCapacity);      // What is the ideal capacity of the battery (how is this expressed)
        wifly.print("/");
        wifly.print(batteryActualCapacity);     // What is the actaul capacity of the battery (is this powerlevel?)
        wifly.print("/");
        wifly.print(batteryBits);               // Other battery information from the POC
        wifly.print("/");
                                              
        // GPS 
        wifly.print(gpsDataType);               // GPS Data 0= current data from gps, 1= cached data, 2= test data       
        wifly.print("/"); 
        wifly.print(gpsStatus);                 // What is the status of the GPS unit (offline, online, other)
        wifly.print("/");
        wifly.print(gpsLongitude);              // Longitude of the device
        wifly.print("/");
        wifly.print(gpsLatitude);               // Latitude of the device
        wifly.print("/");
        wifly.print(gpsAltitude);               // Altitude of the device
        wifly.print("/");
        wifly.print(gpsTime);                   // The timestamp from the GPS stream
        wifly.print("/");
        wifly.print(gpsDate);                   // The date from the GPS Stream
        wifly.print("/");
        wifly.print(gpsFixAge);                 // What is the fixed age
        wifly.print("/");
        wifly.print(gpsAccuracy);               // The accuracy of the gps fix
        wifly.println(" HTTP/1.0");
        
        //Other parameters to wifly
        wifly.print("Host: ");
        wifly.println(lzSite);
        wifly.println("User-Agent: SMART-DME/0.0.1");
        wifly.print("Accept: ");
        wifly.println(lzAccept);
        wifly.print("Accept-Language: ");
        wifly.println(lzAcceptLang);
        wifly.println("Connection: keep-alive");
        wifly.println("Cache-Control: max-age=0");
        wifly.println();   
    } 
    
    else {
        Serial.println("Failed to connect");
    }
}

void loop()
{
    if (wifly.available() > 0) {
	char ch = wifly.read();
	Serial.write(ch);
	if (ch == '\n') {
	    /* add a carriage return */ 
	    Serial.write('\r');
	}
    }

    if (Serial.available() > 0) {
	wifly.write(Serial.read());
    }
}

/* Connect the WiFly serial to the serial monitor. */
void terminal()
{
    while (1) {
	if (wifly.available() > 0) {
	    Serial.write(wifly.read());
	}


	if (Serial.available() > 0) {
	    wifly.write(Serial.read());
	}
    }
}
/* accepts a char[] argument and returns a char[] */
char* stripSemi(char* str)                                        
{
  
 char *out = str, *put = str;

  for(; *str != '\0'; ++str)
  {
    if(*str != ':')
      *put++ = *str;
  }
  *put = '\0';

  return out;
                                               
}

// Converting from Hex to Decimal:

unsigned long hexToDec(String hexString) {
  
  unsigned long decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}

static bool populateGPS()
{
    /* Set GPS */
    Serial.println("Initializing GPS....");
    gpsSerial.begin(9600);
    Serial.print("TinyGPS v:"); 
    Serial.println(TinyGPS::library_version());
    Serial.print("Sizeof(gpsobject) = "); 
    Serial.println(sizeof(TinyGPS));
        
    bool gpsNewData = false;
                                          
    //populateGPS();
   if (gpsSerial.available() > 0) {
        
      if (gps.encode(gpsSerial.read())){
        gpsNewData = true;
      }
      
      Serial.print("New Data: ");
      Serial.print(gpsNewData);
   } 
   if (gpsNewData == true){
      gpsStatus = 0;
      /*GPS Data Type
      0=GPS
      1=Cache
      2=debug
      3=none
      */
      gpsDataType = 3;
      
      gps.f_get_position(&gpsLatitude, &gpsLongitude, &gpsFixAge);         // Get position
      Serial.print("lat: ");
      Serial.print(gpsLatitude);
      Serial.print(" / long: ");
      Serial.print(gpsLongitude);
      Serial.print(" / FixedAge: ");
      Serial.print(gpsFixAge); 
      if (gpsLatitude > 0){
        gpsDataType=3;
      }
      else {
        gpsDataType=0;
      }
      gpsAltitude = gps.f_altitude();           // Altitude of the device
      Serial.print(" / Altitude: ");
      Serial.print(gpsAltitude); 
      gpsAccuracy = 99;                        // The accuracy of the gps fix
        
      gps.stats(&chars, &sentences, &failed);
   }
   else{
     Serial.println("GPS RECEIVE ERROR");
     /*
    Sats HDOP Latitude Longitude Fix  Date       Time       Date Alt     Course Speed Card  Distance Course Card  Chars Sentences Checksum
              (deg)    (deg)     Age                        Age  (m)     --- from GPS ----  ---- to London  ----  RX    RX        Fail
    --------------------------------------------------------------------------------------------------------------------------------------
    5    378  47.76168 -122.33558315  12/04/2013 09:10:24   337  126.30  18.62  1.37  NNE   7687     34.31  NE    2165  10        0        
    5    379  47.76167 -122.33559321  12/04/2013 09:10:29   343  126.40  18.62  0.91  NNE   7687     34.31  NE    4330  20        0        
    5    381  47.76168 -122.33558337  12/04/2013 09:10:34   359  126.60  18.62  0.35  NNE   7687     34.31  NE    6495  30        0        
    */
     gpsStatus = 1; 
     // cache corrdinates // TODO read and write from sd (cut due to sketch size)
     gpsLatitude = seaLat;
     gpsLongitude = seaLong;
     gpsAltitude = seaAlt;
     
      
   }
   Serial.print(" / Status: ");
   Serial.println(gpsStatus); 
}
