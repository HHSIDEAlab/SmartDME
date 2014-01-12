delimiter $$

CREATE TABLE `user_friends` (
  `recordID` int(11) NOT NULL,
  `userFriendID` int(11) DEFAULT NULL,
  `fbFriendID` bigint(20) DEFAULT NULL,
  `firstName` varchar(45) CHARACTER SET latin1 DEFAULT NULL,
  `lastName` varchar(45) CHARACTER SET latin1 DEFAULT NULL,
  `email` varchar(255) CHARACTER SET latin1 DEFAULT NULL,
  `cellPhone` varchar(20) CHARACTER SET latin1 DEFAULT NULL,
  `contactPref` int(11) DEFAULT NULL,
  `smsVerified` int(11) DEFAULT '0',
  `smsRequestSent` int(11) DEFAULT '0',
  `fbInstalled` tinyint(1) DEFAULT '0',
  `fbRequestSent` int(11) DEFAULT '0',
  `fbID` bigint(20) DEFAULT NULL,
  `fbVerified` int(11) DEFAULT '0',
  PRIMARY KEY (`recordID`)
) ENGINE=InnoDB DEFAULT CHARSET=ascii$$

