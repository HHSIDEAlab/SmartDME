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
//#include <MD5.h>       // Crypto
#include <SoftwareSerial.h> // Serial communications
//#include <SD.h>    // SD Card
//#include <avr/sleep.h> // sleep hardware

/* setup debug mode */
#define DEBUG true  //turn debugging on / off
#define DEBUGSD false  //debug to sd card verse serial terminal

// power saving modes
#define SLEEPDELAY 0    /* power-down time in seconds. Max 65535. Ignored if TURNOFFGPS == 0 */
#define TURNOFFGPS 0    /* set to 1 to enable powerdown of arduino and GPS. Ignored if SLEEPDELAY == 0 */
#define LOG_RMC_FIXONLY 0  /* set to 1 to only log to SD when GPD has a fix */

// outgoing communication 
//#define SERVER_PROTOCOL 0 /* 0=ignition, 1=JSON, 2 HTTP-POST
 
uint8_t SERVER_PROTOCOL = 1

/* Setup alamode serial communication ports */
SoftwareSerial gpsSerial(6, 4);
SoftwareSerial wifiSerial(7,8);

/*initize objects */
TinyGPS gps;
WiFly wifly;
//File dataFile;
//File logFile;

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

//Global Varibles (yes they are bad)

String lzParamStr = " ";
String lzContentType = "";
String lzAccept = " ;
            
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
#define chipSelect 10

/* Change these to match your WiFi network */
const char mySSID[] = "Smart-DME";
const char myPassword[] = "SMARTDME";

const char site[] = "ita2.seas.ucla.edu/~fsanborn/firstresponder/";
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
    /*if (!SD.begin(chipSelect)) {
      Serial.println("Card init. failed!");
      error(1);
    }
    */
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

    wifly.setDeviceID("Wifly-WebClient");
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
  
  // need to convert wiFly Mac string to char 
  char charMacBuff[wiFlyMAC.length()];
  wiFlyMAC.toCharArray(charMacBuff, wiFlyMAC.length());
  
   //generate the MD5 hash for our string
  // unsigned char* deviceHash=MD5::make_hash(charMacBuff); //md5 library 
  
  //generate the digest (hex encoding) of our hash
  // char *md5str = MD5::make_digest(deviceHash, 16);
  
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
  
  // Every second we print an update
    while (millis() - start < 10000)
    {
      if (feedgps())
        newdata = true;
    }
     if (wifly.open(site, 80)) {
        Serial.print("Connected to ");
	Serial.println(site);
      /*
            
      // Device Information
      deviceHash                // Unique identifer (mac address for now)
      deviceID                  // Device ID from POC
      devicePowerStatus         // Power Status from POC
      deviceDateTime            // Internal Device Date and Time
      deviceBlob                // Other device information to be used in the future
      deviceHasBattery          // Does the device have a battery
      
      // Battery
      batteryID                 // Battery ID from POC
      batteryPowerLevel         // Powerlevel of the Battery (0-100)
      batteryTemp               // Time to empty (need to determine time unit)
      batteryAvgTimeToEmpty     // Average time to empty (how is this calculated)
      batteryAvgCurrent         // Average Current (over what time)
      batteryAvgVoltage         // Average voltage (over what time)
      batteryCycleCount         // The number of battery cycles (from when?)
      batteryAccessCount        // The number of time the battery has been read (from when?)
      batteryReadCount          // The number of read counts (need clarification)
      batteryIdealCapacity      // What is the ideal capacity of the battery (how is this expressed)
      batteryActualCapacity     // What is the actaul capacity of the battery (is this powerlevel?)
      batteryBits               // Other battery information from the POC
      // stored in the database not part of the protocol set on the server
      batteryTempUnit           // The unit temprature is expressed (1-F, 2-C)
      batteryTimeUnit           // What the encoding format of time (this should be device info)
                                            
      // GPS                   
      gpsStatus                 // What is the status of the GPS unit (offline, online, other)
      gpsLongitude              // Longitude of the device
      gpsLatitude               // Latitude of the device
      gpsAltitude               // Altitude of the device
      gpsTime                   // The timestamp from the GPS stream
      gpsFixAge                 // What is the fixed age
      gpsAccuracy               // The accuracy of the gps fix

      
      https://ita2.seas.ucla.edu/~fsanborn/firstresponder/index.php/
      testing/ 
      add/
          HASH1234/
          ID01/
          1/
          2013-09-09T14:31:03/
          BLOB/
          1/
          BatID/
          100/
          104/
          00.00:00/
          5/
          6/
          100/
          200/
          300/
          90/
          80/
          BATBITS/
          1/
          47.700614/
          -122.35021/
          1005/
          2012-10-29T14:31:03/
          10
      
      */
        //determine the format to send the data to the server in 
        
        switch (SERVER_PROTOCOL) {
                  
          case 0:
            String lzParamStr = "\"testing\"";
            lzParamStr = lzParamStr + "device_token\"";
            String lzContentType = "application/json";
            String lzAccept = " ";
            break;
          
          case 1:
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
*/
            String paramStr = "{\"checkin\":{";
            lzParamStr = lzParamStr + "\device_token\": \"122\",";
            lzParamStr = lzParamStr + "\"card_token\": \"12312\",";
            lzParamStr = lzParamStr + "\"timestamp\": \"2012-10-29T14:31:03\"}}";
        
            break;
          
          case 2:
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
            */

            String paramStr = "{\"checkin\":{";
            lzParamStr = lzParamStr + "\device_token\": \"122\",";
            lzParamStr = lzParamStr + "\"card_token\": \"12312\",";
            lzParamStr = lzParamStr + "\"timestamp\": \"2012-10-29T14:31:03\"}}";
            break;
            
          default: 
            // if nothing else matches, do the default
            // default is optional
        }
        
       Serial.println(paramStr); 
       
         
        /*determine the parameters length */
        String paramLen = String(lzParamStr.length());
	
        /* Send the request */
	wifly.println("POST / HTTP/1.1");
	wifly.print("HOST: ");
        wifly.println(site[]);
	wifly.print("Content-Type: ");
        wifly.println(lzContentType);
        if (SERVER_PROTOCOL == 0){
  	  wifly.println("Accept: ");
          wifly.println(lzAccept);
        }
	wifly.print("Content-Length: ");
        wifly.println(paramLen);
	wifly.println("User-Agent: SMART-DME/0.0.1");
        wifly.println();
	wifly.println(lzParamStr);
	wifly.println();
        
        
    } else {
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
