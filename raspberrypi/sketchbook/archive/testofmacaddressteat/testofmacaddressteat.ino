/* firmware updater
------------------------------------
$$$
set wlan sssid XXX // XXX = SSID
set wlan pass XXX // XXX = PW
save
join
set ftp address 0
set dns name RN.MICROCHIP.COM 
ftp update XXX // XXX = image name (described below)

reboot
$$$
factory R
reboot
------------------------------------
*/

#include <softwareSerial.h>
SoftwareSerial wifiSerial(7,8);
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
    
   /* See if the device is connected */
    if (wifly.isConnected()) {
        Serial.println("Old connection active. Closing");
	wifly.close();
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
    
    /* Open the connection to the server */
    if (wifly.open(lzSite, intPort)) {
        Serial.print("Connected to ");
	Serial.println(lzSite);
                
        //fixed Data for testing
        char* lztmpDeviceHash = wifly.getMAC(wiFlyBuf, sizeof(wiFlyBuf));  // Unique identifer (mac address for now)
        lztmpDeviceHash = stripSemi(lztmpDeviceHash); 
        long  deviceHash = hexToDec(lztmpDeviceHash);
       
        // Device Information
        Serial.print(deviceHash);


//send to wifly
        wifly.print(deviceHash);
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
