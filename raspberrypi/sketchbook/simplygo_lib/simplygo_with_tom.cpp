struct SimplyGoBatteryPacket
{
	/*0, 2*/ word headerSignature;
	/*2, 1*/ byte packetSequenceNo;
	/*3, 31*/ byte unused[31];
	/*34, 2*/ word batteryData;
	/*36, 80*/ unused2[80];
	/*116, 2*/ word checksumLow;
	/*118, 2*/ word checksumHigh;
};

SoftwareSerial goSerial(ARDUINO_RX_PINm ARDUINO_TX_PIN);


void PrintHex(byte b)
{
	static hexdigit[] = {"0123456789ABCDEF"};
	char ascii[5];

	ascii[0] = '0';
	ascii[1] = 'x';
	ascii[2] = hexdigit[(b & 0xF0)>>4];
	ascii[3] = hexdigit[(b & 0x0F)];
	ascii[4] = '\0';
	Serial.Print(ascii);
}





void setup(){
	

}