/*
  
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
 4.	Read word at word offset 15 to get Packet Modulo Count [1024]
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

/* setup debug mode */
#define DEBUG false  //turn debugging on / off

#define PACKET_BYTES    128
#define CAP_BUFF_SIZE   128
#define BYTE_COUNT      512
#define PACKET_CAPTURE_COUNT 5

// What we need to capture
int intCaptureCount = 0;
const int intCaptureMaxCount = CAP_BUFF_SIZE;
byte byteCaptureBuff[CAP_BUFF_SIZE];

// number Packets in the stream
int intPacketCaptureCount = 0;


bool bByteFound = false;
bool bCapture = false;
bool bCaptureDone = false;


// Data Types used in the packet Detail structure
#define DT_BIT     1
#define DT_LNIBBLE 4
#define DT_HNIBBLE 4
#define DT_BYTE    8
#define DT_WORD    16
#define DT_DWORD   32

// Data Packet Type 
#define PT_SYSTEM 2
#define PT_LPC 15

//State transitions
#define STATE_INITIAL 0
#define STATE_WAITING_TO_CAPTURE 1
#define STATE_FOUND_SIGNATURE_LOW 2
#define STATE_CAPTURING 4
#define STATE_CAPTURE_DONE 5
#define STATE_PROCESSING 6
#define STATE_TERMINAL 7

//Define Header info
#define HEADER_START_BYTE_LOW   0xFE
#define HEADER_START_BYTE_HIGH  0x01

//Define Packet Tail
#define TAIL_BYTE_LOW           0x46
#define TAIL_BYTE_HIGH          0x00
#define NULL 0

// Define Packet Offsets
#define MODULO_BYTE_OFFSET            3
#define LPC_MODULO_WORD_OFFSET        15
#define SERIAL_NUMBER_WORD_OFFSET     2
#define MODEL_NUMBER_WORD_OFFSET      2
#define FIRMWARE_WORD_OFFSET          41
#define EXTERNAL_VOLTAGE_WORD_OFFSET  11
#define EXTERAL_AMPERAGE_WORD_OFFSET  40
#define BATTERY_INFO_WORD_OFFSET      34
#define CHECKSUM_LOW_WORD_OFFSET      58
#define CHECKSUM_HIGH_WORD_OFFSET     59



// packet detail structure
typedef struct sgPacket
{
  int  packetType;
  int  type;
  int offset;
  int modulo1;
  int modulo2;
  int modulo3;
  int modulo4;
  bool isSet;
  union {
    word wvalue;
    byte bvalue[8];
  };
} 
SimplyGoPacket;

typedef struct sgData
{
  SimplyGoPacket serialNumber       =  {
    PT_SYSTEM, DT_WORD, 2, 110, 0, 0, 0, false  };  //128 bytes total DWORD
  SimplyGoPacket modelName          =  {
    PT_SYSTEM, DT_BYTE, 2, 110, 0, 0, 0, false  };
  SimplyGoPacket firmwareVersion    =  {
    PT_LPC, DT_WORD, 41, 0, 0, 0, 0, false  };
  SimplyGoPacket externalVoltage    =  {
    PT_LPC, DT_WORD, 11, 0, 0, 0, 0, false  };
  SimplyGoPacket externalAmperage    =  {
    PT_LPC, DT_WORD, 40, 0, 0, 0, 0, false  };
  SimplyGoPacket batteryID          =  {
    PT_LPC, DT_WORD, 34, 6, 262, 518, 774, false  };
  SimplyGoPacket batTemp            =  {
    PT_LPC, DT_WORD, 34, 0, 256, 512, 768, false  };
  SimplyGoPacket batTime2E          =  {
    PT_LPC, DT_WORD, 34, 1, 257, 513, 769, false  };
  SimplyGoPacket batPowerLevel      =  {
    PT_LPC, DT_WORD, 34, 2, 258, 514, 770, false  };
  SimplyGoPacket batAvgCurrent      =  {
    PT_LPC, DT_WORD, 34, 3, 259, 515, 771, false  };
  SimplyGoPacket batAvgVolt         =  {
    PT_LPC, DT_WORD, 34, 4, 260, 516, 772, false  };
  SimplyGoPacket batIdealCap        =  {
    PT_LPC, DT_WORD, 34, 9, 265, 521, 777, false  };
  SimplyGoPacket batActualCap       =  {
    PT_LPC, DT_WORD, 34, 10, 266, 522, 778, false  };
  SimplyGoPacket batBits            =  {
    PT_LPC, DT_WORD, 34, 15, 271, 527, 783, false  };
} 
SimplyGoData;

typedef struct sdmeMonitorData
{
  word serialNumber;    
  word modelName;       
  word firmwareVersion;
  word externalVoltage;
  word externalAmperage;
  word bateryID ;
  word batTemp ;
  word batTime2E ;
  word batPowerLevel;
  word batAvgCurrent ;
  word batAvgVolt ;
  word batIdealCap ;
  word batActualCap ;
  word batBits   ;
  word checkSumLow ;
  word checkSumHigh ; 
  bool isReady;

} 
sdmeMonitorData;

typedef struct sgBatBits
{
  bool plugin           = false;  // bit 0
  bool hasBattery       = false;  // bit 1
  bool canCharge        = false;  // but 2
  bool isCharging       = false;  // bit 3
  bool fullCharge       = false;  // bit 4
  bool powerFail        = false;  // bit 5
  bool alarmInhibited   = false;  // bit 6
  bool srUnderRange     = false;  // bit 7
  bool srHot            = false;  // bit 8
  bool srOverRange      = false;  // bit 9
  bool srCold           = false;  // bit 10
  bool voltageOverRange = false;  // bit 11
  bool currentOverRange = false;  // bit 12
  bool NA1              = false;  // bit 13
  bool NA2              = false;  // bit 14
  bool NA3              = false;  // bit 15
} 
SimplyGoBatteryBits;

SimplyGoData sgData;
SimplyGoPacket sgPacket;
SimplyGoBatteryBits sgBatBits;

int state = STATE_INITIAL;

// the setup routine runs once when you press reset:
void setup() {

  Serial.begin(115200);
  state = STATE_INITIAL;
}

// the loop routine runs over and over again forever:
void loop() 
{
  bool bisSerialAvailable = (Serial.available() > 0);
  byte ch;

  switch(state)
  {  
  case STATE_INITIAL:
    {
      Serial.println("initializing.....");
      state = STATE_WAITING_TO_CAPTURE;
    }
    break;

  case STATE_WAITING_TO_CAPTURE:
    {
      static bool lah = 0;
      if (!lah)
      {
        Serial.println("state wating_to_capture");
        lah = 1;
      }

      if (bisSerialAvailable)
      {
        // check for the start byte
        if (Serial.read() == HEADER_START_BYTE_LOW)
        {
          state = STATE_FOUND_SIGNATURE_LOW;
          lah = 0;
        }
      }
    }
    break;

  case STATE_FOUND_SIGNATURE_LOW:
    {
      static bool lah = 0;
      if (!lah)
      {
        Serial.println("state found_signature_low");
        lah = 1;
      }

      if (bisSerialAvailable)
      {
        // check for the start byte
        if (Serial.read() == HEADER_START_BYTE_HIGH)
        {
          Serial.println("found signature");
          byteCaptureBuff[0] = HEADER_START_BYTE_LOW;
          byteCaptureBuff[1] = HEADER_START_BYTE_HIGH;
          intCaptureCount = 2;
          state = STATE_CAPTURING;

          lah = 0;
        }
        else
        {
          state = STATE_WAITING_TO_CAPTURE;

          lah = 0;
        }
      }
    }
    break;

  case STATE_CAPTURING:
    {
      if (bisSerialAvailable)
      {
        static bool first = 0;
        if (!first)
        {
          Serial.println("state capturing..");
          first = 1; 
        }

        ch = Serial.read();

        // put the incoming byte into the bytebuff
        byteCaptureBuff[intCaptureCount++] = ch;

        if (intCaptureCount == intCaptureMaxCount)
        {
          if (DoesChecksumMatch(byteCaptureBuff))
          {
            state = STATE_PROCESSING;
          }
          else
          {
            intCaptureCount = 0;
            state = STATE_WAITING_TO_CAPTURE;            
          }     
        }  
      }
    }
    break;

  case STATE_PROCESSING:
    {
      Serial.println("state_processing");
      bool bResult = processPacket(byteCaptureBuff);
      intCaptureCount = 0;
      intPacketCaptureCount++; 

      if (intPacketCaptureCount == PACKET_CAPTURE_COUNT)
      {
        state = STATE_TERMINAL;
        intPacketCaptureCount = 0;
      }
      else
        if (bResult == true)
        {
          state = STATE_WAITING_TO_CAPTURE;
        }
        else
        {
          Serial.print("Error in Processing: ");
          Serial.println("");
        }    

    }  
    break;


  case STATE_TERMINAL:
    {

    }
    break;
  }
}

bool DoesChecksumMatch(byte *inBuff)
{
  long *inBuffAsDwordArray = (long *)inBuff;
  long packetChecksum = inBuffAsDwordArray[29];
  long computedSum = 0;
  
  inBuffAsDwordArray[29] = 0x77767574;
  
  
  for (int i = 0; i < 32; i++)
  {
      computedSum += inBuffAsDwordArray[i];
  }
  
  inBuffAsDwordArray[29] = packetChecksum;
  return (computedSum == packetChecksum);
}

void discardBuffer(byte *byteBuff){

  for (int i = 0; i < sizeof(byteBuff); i++){
    byteBuff[i] = 0x00;
  }
}

bool processPacket(byte *inBuff){

  bool bReturn = false;
  word *inBuffAsWordArray = (word*)inBuff;

  // External Voltage
  if (sgData.externalVoltage.isSet == false){
    sgData.externalVoltage.wvalue = inBuffAsWordArray[sgData.externalVoltage.offset];
    //sgData.externalVoltage.isSet = true;
    Serial.print("External Voltage: ");
    Serial.println(sgData.externalVoltage.wvalue, DEC);
  }

  // External Amperage
  if (sgData.externalAmperage.isSet == false){
    ;
    sgData.externalAmperage.wvalue = inBuffAsWordArray[sgData.externalAmperage.offset];
    //sgData.externalAmperage.isSet = true;
    Serial.print("External Amperage: ");
    Serial.println(sgData.externalAmperage.wvalue, DEC);
  }

  // Firmware Version
  if (sgData.firmwareVersion.isSet == false){
    sgData.firmwareVersion.wvalue = inBuffAsWordArray[sgData.firmwareVersion.offset];
    //sgData.firmwareVersion.isSet = true;
    Serial.print("Firmware Version: ");
    byte N = (sgData.firmwareVersion.wvalue >> 12);
    byte M = (sgData.firmwareVersion.wvalue >> 8) & 0x000F;
    byte n = (sgData.firmwareVersion.wvalue >> 4) & 0x000F;
    byte m = (sgData.firmwareVersion.wvalue & 0x000F);
    Serial.print(N, HEX);
    Serial.print(M, HEX);
    Serial.print(n, HEX);
    Serial.print(m, HEX);
    Serial.println("");
  }

  word mbo = inBuffAsWordArray[LPC_MODULO_WORD_OFFSET];
  Serial.print("LPC MODULO COUNT: ");
  Serial.println(mbo, DEC);

  if (sgData.batteryID.modulo1 == mbo || sgData.batteryID.modulo2 == mbo || sgData.batteryID.modulo3 == mbo)
  {
    sgData.batteryID.wvalue = inBuffAsWordArray[sgData.batteryID.offset];
    Serial.print("Battery ID: ");
    Serial.println(sgData.batteryID.wvalue, HEX);
  }




#ifdef DISABLE
  // Serial number
  if (sgData.serialNumber.isSet == false){
    sgData.serialNumber.wvalue = (char*)"123456";
    sgData.serialNumber.isSet = true;
  }

  // model name
  if (sgData.modelName.isSet == false){
    sgData.modelName.wvalue = (char*)"SimplyGo";
    sgData.modelName.isSet = true;
  }
  Serial.print("Serial Number: ");
  Serial.println(*(word*)sgData.serialNumber.value);
  Serial.print("Model Name: ");
  Serial.println((char*)sgData.modelName.value);
#endif //DISABLE 



  // External Voltage (used to determine if plugged in 

  // Battery Information

  // External Amperage 

    // Internal Software Version

  // Checksum Low

  // Checksum High

  //if (DEBUG == true){
  for (int x = 0; x < CAP_BUFF_SIZE; x++){
    Serial.print(inBuff[x], HEX);
    Serial.print(",");
  } 
  Serial.println("");

  bReturn = true;

  return bReturn; 
} 


