delimiter $$

CREATE TABLE `sdme_collector` (
  `recordID` int(11) NOT NULL AUTO_INCREMENT,
  `recordTime` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `deviceDataType` int(11) DEFAULT NULL COMMENT 'This is the main data collector for the SMARTDME device',
  `deviceHash` varchar(45) DEFAULT NULL,
  `deviceID` varchar(255) DEFAULT NULL,
  `deviceFirmwareVersion` varchar(6) DEFAULT NULL,
  `deviceDateTime` datetime DEFAULT NULL,
  `devicePowerStatus` int(11) DEFAULT NULL,
  `deviceHasBattery` tinyint(1) DEFAULT NULL,
  `deviceBlob` blob,
  `sdmeDataType` int(11) DEFAULT '1',
  `sdmePowerLevel` int(11) DEFAULT NULL,
  `sdmeVoltage` int(11) DEFAULT NULL,
  `sdmeCurrent` int(11) DEFAULT NULL,
  `batteryID` varchar(50) DEFAULT NULL,
  `batteryPowerLevel` int(11) DEFAULT NULL,
  `batteryTemp` int(11) DEFAULT NULL,
  `batteryAvgTimeToEmpty` int(11) DEFAULT NULL,
  `batteryAvgCurrent` int(11) DEFAULT NULL,
  `batteryAvgVoltage` int(11) DEFAULT NULL,
  `batteryCycleCount` int(11) DEFAULT NULL,
  `batteryAccessCount` int(11) DEFAULT NULL,
  `batteryReadCount` int(11) DEFAULT NULL,
  `batteryIdealCapacity` int(11) DEFAULT NULL,
  `batteryActualCapacity` int(11) DEFAULT NULL,
  `batteryBits` varchar(16) DEFAULT NULL,
  `batteryTempUnit` int(11) DEFAULT NULL,
  `batteryTimeUnit` int(11) DEFAULT NULL,
  `gpsDataType` int(11) DEFAULT NULL,
  `gpsStatus` int(11) DEFAULT NULL,
  `gpsLongitude` float(10,6) DEFAULT NULL,
  `gpsLatitude` float(10,6) DEFAULT NULL,
  `gpsAltitude` int(11) DEFAULT NULL,
  `gpsTime` time DEFAULT NULL,
  `gpsDate` date DEFAULT NULL,
  `gpsFixAge` int(11) DEFAULT NULL,
  `gpsAccuracy` int(11) DEFAULT NULL,
  PRIMARY KEY (`recordID`),
  UNIQUE KEY `recordID_UNIQUE` (`recordID`)
) ENGINE=InnoDB AUTO_INCREMENT=113 DEFAULT CHARSET=latin1 COMMENT='SmartDME Collector'$$

