/*
 
 This example code is in the public domain.
 
/*
SimplyGo device implementation

DeviceCategory               The category of Device (predefined list) *Should Be stored on the server
DeviceManufacture            The device manfucture or the hash of the manufacture *should be stored on the server
DeviceModel                  The device model or hash *should be stored on the server
----------------------------------------------------------
DeviceHash                   The Hash of the device (unique device ID -- hash of MacAddress)
DeviceID					 The ID of the device from the manufacture
DevicePowerStatus            The current power status (stationary, battery, unknown)
DeviceDateTime               Device Date and Time (year, month, day, hour, minute, second, hundredths) 
HasBattery                   Boolean (true /false)
DevicePowerLevel             The current battery level (0-100)
BatteryID                    The ID of the battery
BatteryTemp                  The Temperature of the battery (Need unit measurement)
BatteryAvgTimeToEmpty        The time to empty
BatteryAvgCurrent			 The average current
BatteryAvgVoltage			 The average votage
BatteryCycleCount			 The cycle count of the battery
BatteryAccessCount			 The number of time the battery has been accessed
BatteryReadCount			 The number of time the battery has been read
BatteryIdealCapacity         The ideal capacity as determined by the device
BatteryActualCapacity		 The actual capacity of the battery
BatteryBits					 Extra battery bit as defined the manufacture
.

/*
Coming From the SimplyGo Device

 Each packet contains 128 bytes and contains the following data.
	2 Header bytes that allow Host synchronization with the packet (0x01FE)
	A 1-byte Packet Sequence Number (at word offset 1) that is incremented each time a packet is transmitted;   aka <0, 1, 2…255, 0, 1, 2...>. This facilitates the Host in identifying missing packets. 
	All the Data
	Bytes 116, 117, 118 & 119 contain a 32-bit Checksum of the remaining 124 bytes contained in the packet. Bytes at 116 and 117 are the LSW.
 
Since the broadcast packets are only 128 bytes and we have more than 128 bytes of system data, we send different data in each packet. The Packet Modulo Count is used to indicate which data is contained in the packet.
 
Example for extracting battery and power usage data:
1.	Capture 128 byte Packet (word at word offset 0 is 0x01FE)
2.	Read word at word offset 27 to get Motor  Voltage scaled by 10
3.	Read word at word offset 28 to get Motor current (milliamps)
4.	Read word at word offset 15 to get Packet Modulo Count
5.	Read word at word offset 34 to get battery data
a.	If Packet Modulo Count = 0, 31, 63, .. the battery data = battery temperature
b.	If Packet Modulo Count = 1, 32, 64, .. the battery data = battery average time to empty
c.	If Packet Modulo Count = 2, 33, 65, .. the battery data = battery % Charge
d.	If Packet Modulo Count = 3, 34, 66, .. the battery data = battery Avg. Current
e.	If Packet Modulo Count = 4, 35, 67, .. the battery data = battery Avg. voltage
f.	If Packet Modulo Count = 5, 36, 68, .. the battery data = battery cycle count
g.	If Packet Modulo Count = 6, 37, 69, .. the battery data = battery ID
h.	If Packet Modulo Count = 7, 38, 70, .. the battery data = Number of battery accesses
i.	If Packet Modulo Count = 8, 39, 71, .. the battery data = Number of battery reads
j.	If Packet Modulo Count = 9, 40, 72, .. the battery data = battery ideal capacity
k.	If Packet Modulo Count = 10, 41, 73, .. the battery data = battery actual full capacity
l.	If Packet Modulo Count = 15, 46, 78, .. the battery data = battery mode bits

 */
#ifndef __SDME_SIMPLYGO_H__
#define __SDME_SIMPLYGO_H__

/*
Arduino Simplygo Device Driver
Compatible with Arduino 1.0
Version .5

SoftwareSerial is exposed as serial i/o

Credits:
  SoftwareSerial   Mikal Hart   http://arduiniana.org/
  Time             Michael Margolis http://www.arduino.cc/playground/uploads/Code/Time.zip

Copyright HHS.GOV 2013
*/



#include <SoftwareSerial.h>

//Set Communication PINS
#define ARDUINO_RX_PIN  2
#define ARDUINO_TX_PIN  3


#define RX_BUFFER_SIZE 128


struct
{
DevicePowerLevel             The current battery level (0-100)
BatteryID                    The ID of the battery
BatteryTemp                  The Temperature of the battery (Need unit measurement)
BatteryAvgTimeToEmpty        The time to empty
BatteryAvgCurrent			 The average current
BatteryAvgVoltage			 The average votage
BatteryCycleCount			 The cycle count of the battery
BatteryAccessCount			 The number of time the battery has been accessed
BatteryReadCount			 The number of time the battery has been read
BatteryIdealCapacity         The ideal capacity as determined by the device
BatteryActualCapacity		 The actual capacity of the battery
BatteryBits					 Extra battery bit as defined the manufacture

}

typedef unsigned char BYTE;

struct SimplyGoHeader
{
	/*0, 2*/ word headerSignature;
	/*2, 1*/ byte packetSequenceNo;
};

struct SimplyGoBatteryPacket : public SimplyGoHeader
{
	/*3, 31*/ byte unused[31];
	/*34, 2*/ word batteryData;
	/*36, 80*/ byte unused2[80];
	/*116, 2*/ word checksumLow;
	/*118, 2*/ word checksumHigh;
};

// Utility functions


class GoDebug : public Stream {
public:
    GoDebug();
    void begin(Stream *debugPrint);

    virtual size_t write(uint8_t byte);
    virtual int read() { return debug->read(); }
    virtual int available() { return debug->available(); }
    virtual void flush() { return debug->flush(); }
    virtual int peek() { return debug->peek(); }

    using Print::write;
private:
    Stream *debug;
};

class SimplyGoSerial : public Stream {
  public:
    // Constructors
    SimplyGoSerial(byte pinReceive, byte pinSend);
    
    // Destructor
    
    // Initialization
    boolean begin();  // Initialises this interface Class.
    
    // Status
    
    // Obtain current device status flags
    long    getDeviceStatus();  // refreshes device status flags.
	boolen isAvailable();
    
                
    // Information

	char* getLibraryVersion(char* buf, int buflen);
	char* getDeviceManufacture(char* buf, int buflen);
	char* getDeviceModel(char* buf, int buflen);
	char* getDeviceHash(char* buf, int buflen);
	char* getDeviceID(char* buf, int buflen);
	char* getDevicePowerStatus(char* buf, int buflen);
	char* getDeviceDateTime(char* buf, int buflen);
	char* getHasBattery(char* buf, int buflen);
	char* getBatteryPowerLevel(char* buf, int buflen);
	char* getBatteryID(char* buf, int buflen);
	char* getBatteryTemp(char* buf, int buflen);
	char* getBatteryAvgTimeToEmpty(char* buf, int buflen);
	char* getBatteryAvgCurrent(char* buf, int buflen);
	char* getBatteryAvgVoltage(char* buf, int buflen);
	char* getBatteryCycleCount(char* buf, int buflen);
	char* getBatteryAccessCount(char* buf, int buflen);
	char* getBatteryReadCount(char* buf, int buflen);
	char* getBatteryIdealCapacity(char* buf, int buflen);
	char* getBatteryActualCapacity(char* buf, int buflen);
	char* getBatteryAvgVoltage(char* buf, int buflen);
	char* getBatteryAvgVoltage(char* buf, int buflen);
	char* getBatteryAvgVoltage(char* buf, int buflen);
	bool testMode();
	/*
	TestMode (bool) 	     Will Send Test Data 
	DeviceCategory               The category of Device (predefined list) *Should Be stored on the server
	DeviceManufacture            The device manfucture or the hash of the manufacture *should be stored on the server
	DeviceModel                  The device model or hash *should be stored on the server
	----------------------------------------------------------
	DeviceHash                   The Hash of the device (unique device ID -- hash of MacAddress)
	DeviceID					 The ID of the device from the manufacture
	DevicePowerStatus            The current power status (stationary, battery, unknown)
	DeviceDateTime               Device Date and Time (year, month, day, hour, minute, second, hundredths) 
	HasBattery                   Boolean (true /false)
	BatteryPowerLevel             The current battery level (0-100)
	BatteryID                    The ID of the battery
	BatteryTemp                  The Temperature of the battery (Need unit measurement)
	BatteryAvgTimeToEmpty        The time to empty
	BatteryAvgCurrent			 The average current
	BatteryAvgVoltage			 The average votage
	BatteryCycleCount			 The cycle count of the battery
	BatteryAccessCount			 The number of time the battery has been accessed
	BatteryReadCount			 The number of time the battery has been read
	BatteryIdealCapacity         The ideal capacity as determined by the device
	BatteryActualCapacity		 The actual capacity of the battery
	BatteryBits					 Extra battery bit as defined the manufacture
	*/
    
	// Device Info
    char* getLibraryVersion(char* buf, int buflen);
    unsigned long getTime();
    
	SoftwareSerial simplygoSerial;    
       
        
// Test Mode
    const boolean bCapturing = true;

    // debug utilities - use Serial : not NewSoftSerial as it will affect incoming stream.
    // should change these to use stream <<
    void    setDebugChannel( Print* pDebug);
    Print*  getDebugChannel( )  { return pDebugChannel; };
    void    clearDebugChannel();
    void    DebugPrint( const char* pMessage);
    void    DebugPrint( const int iNumber);
    void    DebugPrint( const char ch);
    
  private:
    
    // Internal status flags
    long    fStatus;        
    char*   ExtractLineFromBuffer(const int idString,  char* pBuffer, const int bufsize, const char* pStartPattern, const char* chTerminator);
    
    // Internal debug channel.
    Print*  pDebugChannel;

};


#endif
