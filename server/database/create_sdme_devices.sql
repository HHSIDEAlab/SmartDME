delimiter $$

CREATE TABLE `sdme_devices` (
  `recordID` int(11) NOT NULL,
  `typeID` int(11) DEFAULT NULL,
  `manufacture` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `model` varchar(255) COLLATE utf8_unicode_ci DEFAULT NULL,
  `hasBattery` int(1) DEFAULT NULL,
  PRIMARY KEY (`recordID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Smart DME Devices'$$

