/*
 * Smart DME signal device - SDMEUpload
 *
 * This sketch implements a simple Web client 
 * that gathers the current time, the gps location, 
 * and the energy levels of the Durable Medical Equipment
 * the sends the information via a POST.
 *
 * This sketch is released to the public domain.
 *
 */


/*
DeviceCategory               The category of Device (predefined list) *Should Be stored on the server
DeviceManufacture            The device manfucture or the hash of the manufacture *should be stored on the server
DeviceModel                  The device model or hash *should be stored on the server
----------------------------------------------------------
DeviceID                     The Hash of the device (unique device ID -- hash of MacAddress)
DevicePowerStatus            The current power status (stationary, battery, unknown)
DeviceDateTime               Device Date and Time (year, month, day, hour, minute, second, hundredths) 
HasBattery                   Boolean (true /false)
DevicePowerLevel             The current battery level (0-100)
DeviceAttributeXMLBlob       A XML Blob that contains other device attribute
----------------------------------------------------------
GPSLongitude                 GPS Longitude
GPSLatitude                  GPS Latitude
GPSAltitude                  GPS Altitude
GPSFixAge                    The number of milliseconds since the last valid GPS fix. 
GPSDateTime                  GPS Date and Time (year, month, day, hour, minute, second, hundredths)
GPSStatus                    The status of the GPS <0=online, 1=offline, 2=unknown)
*/

#include <TinyGPS.h>   //GPS
#include <WiFlyHQ.h>   //Wi-Fi
//#include <MD5.h>       // Crypto  -- Crypto library is 12k way too big for use. 
#include <SoftwareSerial.h> // Serial communications
//#include <SD.h>    // SD Card --cut due to space
//#include <avr/sleep.h> // sleep hardware -- cut due to space

/* setup debug mode */
#define DEBUG true  //turn debugging on / off
#define DEBUGSD false  //debug to sd card verse serial terminal

// power saving modes
#define SLEEPDELAY 0    /* power-down time in seconds. Max 65535. Ignored if TURNOFFGPS == 0 */
#define TURNOFFGPS 0    /* set to 1 to enable powerdown of arduino and GPS. Ignored if SLEEPDELAY == 0 */
#define LOG_RMC_FIXONLY 0  /* set to 1 to only log to SD when GPD has a fix */

/* Setup alamode serial communication ports */
SoftwareSerial gpsSerial(6, 4);
SoftwareSerial wifiSerial(7,8);

/* initize objects */
TinyGPS gps;
WiFly wifly;
//File dataFile; --sd support removed due to space
//File logFile; -- sd support removed due to space

/* setup communication protocol to webserver */
#define WEB_PROTO 2 /* 0 = json, 1= http POST, 2 = code ignite */


/* GPS parameters*/
#define BUFFSIZE 90
char buffer[BUFFSIZE];
uint8_t bufferidx = 0;
bool fix = false; // current fix data
bool gotGPRMC;    //true if current data is a GPRMC strinng
uint8_t i;

// read a Hex value and return the decimal equivalent
uint8_t parseHex(char c) {
  if (c < '0')
    return 0;
  if (c <= '9')
    return c - '0';
  if (c < 'A')
    return 0;
  if (c <= 'F')
    return (c - 'A')+10;
}


// blink out an error code
void error(uint8_t errno) 
{
/*
  if (SD.errorCode()) {
    putstring("SD error: ");
    Serial.print(card.errorCode(), HEX);
    Serial.print(',');
    Serial.println(card.errorData(), HEX);
  }
  */
}

/* SD Card Parameters */
//#define chipSelect 10

/* Change these to match your WiFi network */
const char mySSID[] = "smartDME";
const char myPassword[] = "Smart-DME";

const char site[] = "ita2.seas.ucla.edu";
const int port = 80;
//const char serverHASH[] = "";

static void gpsdump(TinyGPS &gps);
static bool feedgps();
static void print_float(float val, float invalid, int len, int prec);
static void print_int(unsigned long val, unsigned long invalid, int len);
static void print_date(TinyGPS &gps);
static void print_str(const char *str, int len);


void setup()
{
  
    WDTCSR |= (1 << WDCE) | (1 << WDE);
    WDTCSR = 0;
    Serial.begin(9600);
    Serial.println("\r\nGPSlogger");
    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(10, OUTPUT);
    
    // see if the card is present and can be initialized:
    //if (!SD.begin(chipSelect)) {
    //  Serial.println("Card init. failed!");
    //  error(1);
    //}
    
    char buf[32];

    Serial.begin(115200);
    
    Serial.println("Starting");
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
	wifly.setSSID(mySSID);
	wifly.setPassphrase(myPassword);
	wifly.enableDHCP();

	if (wifly.join()) {
	    Serial.println("Joined wifi network");
	} else {
	    Serial.println("Failed to join wifi network");
	    terminal();
	}
    } else {
        Serial.println("Already joined network");
    }

    //terminal();
  
    Serial.print("MAC: ");
    String wiFlyMAC =  wifly.getMAC(buf, sizeof(buf));
    Serial.println(wiFlyMAC);
    Serial.print("IP: ");
    String wiFlyIP = wifly.getIP(buf, sizeof(buf));
    Serial.println(wiFlyIP);
    Serial.print("Netmask: ");
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    Serial.print("Gateway: ");
    Serial.println(wifly.getGateway(buf, sizeof(buf)));

    wifly.setDeviceID("smartDME_Client");
    Serial.print("DeviceID: ");
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    if (wifly.isConnected()) {
        Serial.println("Old connection active. Closing");
	wifly.close();
    }   

  // Setup GPS
  
  gpsSerial.begin(9600);
  
  Serial.print("TinyGPS v:"); Serial.println(TinyGPS::library_version());
  Serial.print("Sizeof(gpsobject) = "); Serial.println(sizeof(TinyGPS));
  
  
  /*  MD5 hashing library is too large for use 
  
  // need to convert wiFly Mac string to char 
  char charMacBuff[wiFlyMAC.length()];
  wiFlyMAC.toCharArray(charMacBuff, wiFlyMAC.length());
  
   //generate the MD5 hash for our string
  unsigned char* deviceHash=MD5::make_hash(charMacBuff);
  
  //generate the digest (hex encoding) of our hash
  char *md5str = MD5::make_digest(deviceHash, 16); 
  
  *///////////
  
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
    
    bool newdata = false;
    unsigned long start = millis();
  
  // Every 5 minutes we print an update
  while (millis() - start < 5000)
  {
    if (feedgps())
      newdata = true;
    }

    if (wifly.open(site, port)) {
        Serial.print("Connected to ");
	Serial.println(site);

      /* GPS Varibles */      
 
      float flat, flon;
      int numSatellites = 0;
      unsigned long age, date, time, chars = 0;
      unsigned short sentences = 0, failed = 0;
      
      numSatellites = gps.satellites();
      gps.f_get_position(&flat, &flon, &age);
          
      print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 8, 2);
      
      gps.stats(&chars, &sentences, &failed);
      print_int(chars, 0xFFFFFFFF, 6);
      print_int(sentences, 0xFFFFFFFF, 10);
      print_int(failed, 0xFFFFFFFF, 9);
      Serial.println();
      
      //convert floats to strings
      
      //dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);
              
        /*
        // sample url to post data
        https://ita2.seas.ucla.edu/~fsanborn/firstresponder/index.php/testing/add/HASH1234/ID01/1/2013-09-09T14:31:03/BLOB/1/BatID/100/104/00.00:00/5/6/100/200/300/90/80/BATBITS/1/47.700614/-122.35021/1005/2012-10-29T14:31:03/10
        
        //HEADER FROM FIREFOX 
        
        GET /~fsanborn/firstresponder/index.php/testing/add/HASH1234/ID01/1/2013-09-09T14:31:03/BLOB/1/BatID/100/104/00.00:00/5/6/100/200/300/90/80/BATBITS/1/47.700614/-122.35021/1005/2012-10-29T14:31:03/10 HTTP/1.1
        Host: ita2.seas.ucla.edu
        User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:23.0) Gecko/20100101 Firefox/23.0
        Accept: text/html,application/xhtml+xml,application/xml;
        Accept-Language: en-US,en;q=0.5
        Accept-Encoding: gzip, deflate
        Connection: keep-alive
        Cache-Control: max-age=0
        */
 
      char* deviceHash = "1234";                       // Unique identifer (mac address for now)
      char* deviceID = "deviceID";                     // Device ID from POC
      char* devicePowerStatus = "1";                   // Power Status from POC
      char* deviceDateTime = "2013-09-09T14:31:03";    // Internal Device Date and Time
      char* deviceBlob = "DeviceBlob";               // Other device information to be used in the future
      char* deviceHasBattery = "1";                    // Does the device have a battery
      
      // Battery
      char* batteryID ="BATID";                        // Battery ID from POC
      char* batteryPowerLevel = "80";                  // Powerlevel of the Battery (0-100)
      char* batteryTemp = "60";                        // Time to empty (need to determine time unit)
      char* batteryAvgTimeToEmpty = "00:00:00";        // Average time to empty (how is this calculated)
      char* batteryAvgCurrent = "5";                   // Average Current (over what time)
      char* batteryAvgVoltage = "6";                   // Average voltage (over what time)
      char* batteryCycleCount = "1000";                // The number of battery cycles (from when?)
      char* batteryAccessCount = "2000";               // The number of time the battery has been read (from when?)
      char* batteryReadCount = "3000";                 // The number of read counts (need clarification)
      char* batteryIdealCapacity = "90";               // What is the ideal capacity of the battery (how is this expressed)
      char* batteryActualCapacity = "80";              // What is the actaul capacity of the battery (is this powerlevel?)
      char* batteryBits = "batterybits";               // Other battery information from the POC
                                            
      // GPS                   
      char* gpsStatus = "1";                           // What is the status of the GPS unit (offline, online, other)
      char* gpsLongitude = "47.700614";                // Longitude of the device
      char* gpsLatitude = "-122.35021";                // Latitude of the device
      char* gpsAltitude = "500";                       // Altitude of the device
      char* gpsTime = "14:31:03";                      // The timestamp from the GPS stream
      char* gpsDate = "2013-09-09";                    // The date fromt he gps stream
      char* gpsFixAge = "10";                          // What is the fixed age
      char* gpsAccuracy = "99";                        // The accuracy of the gps fix
 
        
      // set the content type
      char* lzContentType = "application/html";
      char* lzAcceptLang = "en-us,en";
      char* lzAccept = "text/html,application/xhtml+xml,application/xml;";
      char* lzPostType = "GET ";
      char* lzServerPath = "~fsanborn/firstresponder/index.php/";
      char* lzParam = "testing/add/";
      
      // Device Information
      Serial.print(deviceHash);                // Unique identifer (mac address for now)
      Serial.print("/");
      Serial.print(deviceID);                 // Device ID from POC
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
      Serial.print("HOST: ");
      Serial.println(site);
      Serial.println("User-Agent: SMART-DME/0.0.1");
      Serial.print("Accept: ");
      Serial.println(lzAccept);
      Serial.print("Accept-Language: ");
      Serial.println(lzAcceptLang);
      Serial.print("Content-Length: ");
      Serial.println(lzParamLen);
      Serial.println("Connection: keep-alive");
      Serial.println("Cache-Control: max-age=0");
        
      /* Send the request */
//send to wifly
      wifly.print(lzPostType);
      wifly.print(lzServerPath);
      wifly.print(lzParam);        
      wifly.print(deviceHash);                // Unique identifer (mac address for now)
      wifly.print("/");
      wifly.print(deviceID);                 // Device ID from POC
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
      wifly.print("HOST: ");
      wifly.println(site);
      wifly.println("User-Agent: SMART-DME/0.0.1");
      wifly.print("Accept: ");
      wifly.println(lzAccept);
      wifly.print("Accept-Language: ");
      wifly.println(lzAcceptLang);
      wifly.println("Connection: keep-alive");
      wifly.println("Cache-Control: max-age=0");
      wifly.println("");                    
     }
     
    else {
        Serial.println("Failed to connect");
    }         
                                
  //terminate();
  
  gpsdump(gps);
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

static void gpsdump(TinyGPS &gps)
{
  float flat, flon;
  unsigned long age, date, time, chars = 0;
  unsigned short sentences = 0, failed = 0;
  
  print_int(gps.satellites(), TinyGPS::GPS_INVALID_SATELLITES, 5);
  print_int(gps.hdop(), TinyGPS::GPS_INVALID_HDOP, 5);
  gps.f_get_position(&flat, &flon, &age);
  print_float(flat, TinyGPS::GPS_INVALID_F_ANGLE, 9, 5);
  print_float(flon, TinyGPS::GPS_INVALID_F_ANGLE, 10, 5);
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);

  print_date(gps);

  print_float(gps.f_altitude(), TinyGPS::GPS_INVALID_F_ALTITUDE, 8, 2);
  
  gps.stats(&chars, &sentences, &failed);
  print_int(chars, 0xFFFFFFFF, 6);
  print_int(sentences, 0xFFFFFFFF, 10);
  print_int(failed, 0xFFFFFFFF, 9);
  Serial.println();
}

static void print_int(unsigned long val, unsigned long invalid, int len)
{
  char sz[32];
  if (val == invalid)
    strcpy(sz, "*******");
  else
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  feedgps();
}

static void print_float(float val, float invalid, int len, int prec)
{
  char sz[32];
  if (val == invalid)
  {
    strcpy(sz, "*******");
    sz[len] = 0;
        if (len > 0) 
          sz[len-1] = ' ';
    for (int i=7; i<len; ++i)
        sz[i] = ' ';
    Serial.print(sz);
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1);
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(" ");
  }
  feedgps();
}

static void print_date(TinyGPS &gps)
{
  int year;
  byte month, day, hour, minute, second, hundredths;
  unsigned long age;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
  if (age == TinyGPS::GPS_INVALID_AGE)
    Serial.print("*******    *******    ");
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d %02d:%02d:%02d   ",
        month, day, year, hour, minute, second);
    Serial.print(sz);
  }
  print_int(age, TinyGPS::GPS_INVALID_AGE, 5);
  feedgps();
}

static void print_str(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  feedgps();
}

static bool feedgps()
{
  while (gpsSerial.available())
  {
    if (gps.encode(gpsSerial.read()))
      return true;
  }
  return false;
}


/*
   //determine the format to send the data to the server in 
      
        switch(WEB_PROTO) {
          
          case 0:  //json
            /*
              Serial.print("Connected to ");
              Serial.println(site);
              String params = "{\"checkin\":{\"device_token\": \"122\",\"card_token\": \"12312\", \"timestamp\": \"2012-10-29T14:31:03\"}}";
              String paramsLength = String(params.length());
              wifly.println("POST /checkins HTTP/1.1");
              wifly.println("Host: localhost:3000");
              wifly.println("Content-type: application/json");
              wifly.println("Accept: application/json");
              wifly.print("Content-Length: ");
              wifly.println(paramsLength);
              wifly.println("User-Agent: easyPEP/0.0.1");
              wifly.println();
              wifly.println(params);
           // -- End comment
           
            lzParam = "{\"" + lzRequest + "\":{";
            lzParam = lzParam + "\device_token\": \"122\",";
            lzParam = lzParam + "\"card_token\": \"12312\",";
            lzParam = lzParam + "\"timestamp\": \"2012-10-29T14:31:03\"}}"; 
            
            lzContentType = "application/json";
            lzAccept = "application/json";


            lzParam = "{\"checkin\":{";
            lzParam = lzParam + "\device_token\": \"122\",";
            lzParam = lzParam + "\"card_token\": \"12312\",";
            lzParam = lzParam + "\"timestamp\": \"2012-10-29T14:31:03\"}}";
            break;
            
          case 1:   //HHTP POST
             /*_wifly->print("POST ");
              _wifly->print(request);
              _wifly->println(" HTTP/1.1");
              _wifly->println("Content-Type: application/x-www-form-urlencoded");
              _wifly->println("Connection: close");
              _wifly->print("Host: ");
              _wifly->println(_site);
              _wifly->print("Content-Length: ");
              _wifly->println(strlen(data));
              _wifly->println();
              _wifly->print(data);
            // -- End Comment
            break;
          
          case 2:

*/
/*

static void writeFile(char *fileName)
{
    // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card init. failed!");
    error(1);
  }

  Serial.print("Writing to "); Serial.println(buffer);
    strcpy(buffer, "fileName.txt"); // DEBUG // UPDATE TO VARIBLE FILENAME  
    for (i = 0; i < 100; i++) {
      buffer[6] = '0' + i/10;
      buffer[7] = '0' + i%10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(buffer)) {
        break;
      }
    }
  
    logfile = SD.open(buffer, FILE_WRITE);
    if( ! logfile ) {
      Serial.print("Couldnt create "); Serial.println(buffer);
      error(3);
    }
}
*/
