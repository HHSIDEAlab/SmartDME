/*
 * WiFlyHQ Example httpclient.ino
 *
 * This sketch implements a simple Web client that connects to a 
 * web server, sends a GET, and then sends the result to the 
 * Serial monitor.
 *
 * This sketch is released to the public domain.
 *
 */

#include <WiFlyHQ.h>

#include <SoftwareSerial.h>
SoftwareSerial wifiSerial(7,8);

//#include <AltSoftSerial.h>
//AltSoftSerial wifiSerial(8,9);

WiFly wifly;

/* Change these to match your WiFi network */
const char mySSID[] = "smartDME";
const char myPassword[] = "Smart-DME";

const char site[] = "ita2.seas.ucla.edu";

void terminal();

void setup()
{
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

    char* lzRequest = "GET /~fsanborn/firstresponder/index.php/testing/add/HASH1234/ID01/1/2013-09-09T14:31:03/BLOB/1/BatID/100/104/00.00:00/5/6/100/200/300/90/80/BATBITS/1/47.700614/-122.35021/1005/2012-10-29T14:31:03/10 HTTP/1.1";

    Serial.print("MAC: ");
    Serial.println(wifly.getMAC(buf, sizeof(buf)));
    Serial.print("IP: ");
    Serial.println(wifly.getIP(buf, sizeof(buf)));
    Serial.print("Netmask: ");
    Serial.println(wifly.getNetmask(buf, sizeof(buf)));
    Serial.print("Gateway: ");
    Serial.println(wifly.getGateway(buf, sizeof(buf)));

    wifly.setDeviceID("SmartDME-Client");
    Serial.print("DeviceID: ");
    Serial.println(wifly.getDeviceID(buf, sizeof(buf)));

    if (wifly.isConnected()) {
        Serial.println("Old connection active. Closing");
	wifly.close();
    }

    if (wifly.open(site, 80)) {
        Serial.print("Connected to ");
	Serial.println(site);
        Serial.println(lzRequest);
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
        char* lzPostType = "GET /";
        char* lzServerPath = "~fsanborn/firstresponder/index.php/";
        char* lzParam = "testing/add/";
        
        // Device Information
        Serial.print(lzPostType);
        Serial.print(lzServerPath);
        Serial.print(lzParam);        
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
        Serial.print("/");
        Serial.print(" HTTP/1.1");

        
	/* Send the request */
	//wifly.println(lzRequest);
	//wifly.println("");
    } else {
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
