/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 
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
 #include <SoftwareSerial.h> // Serial communications
 #include <simplygo.h> 

/* setup debug mode */
#define DEBUG true  //turn debugging on / off
#define DEBUGSD false  //debug to sd card verse serial terminal

#define ARDUINO_RX_PIN  2
#define ARDUINO_TX_PIN  3

SoftwareSerial goSerial(ARDUINO_RX_PIN, ARDUINO_TX_PIN);



struct SimplyGoBatteryPacket
{
	/*0, 2*/ word headerSignature;
	/*2, 1*/ byte packetSequenceNo;
	/*3, 31*/ byte unused[31];
	/*34, 2*/ word batteryData;
	/*36, 80*/ byte unused2[80];
	/*116, 2*/ word checksumLow;
	/*118, 2*/ word checksumHigh;
};

void PrintHex(byte b)
{
	const char hexdigit[] = {"0123456789ABCDEF"};
	char ascii[5];

	ascii[0] = '0';
	ascii[1] = 'x';
	ascii[2] = hexdigit[(b & 0xF0)>>4];
	ascii[3] = hexdigit[(b & 0x0F)];
	ascii[4] = 0;
	Serial.println(ascii);
}

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  goSerial.begin(115200);
    
}

// the loop routine runs over and over again forever:
void loop() {
    if (goSerial.available() > 0) {
	byte ch = (byte)goSerial.read();
        if (ch==0xFE){
          PrintHex(ch);
          ch = (byte)goSerial.read();
          PrintHex(ch);
          if (ch==0x01){
            Serial.println("jhfjkdhskfjhkdsjaf");
            
          }
         
        }

        
	//Serial.write(ch);
    }

    
  delay(10);        // delay in between reads for stability
  
}
