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

wifly-EZX.img = Ad Hoc mode support ver. 2.38
wifly-EZX-AP.img = Soft AP mode support ver. 2.42
wifly7-400.mif = Version 4.0 latest


*/

#include <SoftwareSerial.h>
SoftwareSerial wifly(7,8);

void setup(){
//send to wifly
        Serial.println("Upgrade Starting...");
        wifly.println("$$$");
        Serial.println("Join network");
        wifly.println("set wlan sssid Smart-DME");
        wifly.println("set wlan pass SmartDME");        
        wifly.println("save");
        wifly.println("join");
        Serial.println("Set ftp..");
        wifly.println("set ftp address 0");   
        wifly.println("set dns name RN.MICROCHIP.COM");
        //wifly.println("ftp update XXX");  
        Serial.println("Downloading.... ");
        wifly.println("ftp update wifly7-400.mif");   
        wifly.println("");
        Serial.println("rebooting....");
        wifly.println("reboot");           
        wifly.println("$$$");
        Serial.println("reset factory defaults");
        wifly.println("factory R");    
        wifly.println("reboot");
        
        Serial.println("Upgrade Complete");
  }
  
  void loop(){
    
  }

