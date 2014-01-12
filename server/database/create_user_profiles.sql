delimiter $$

CREATE TABLE `user_profiles` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `country` varchar(20) COLLATE utf8_bin DEFAULT NULL,
  `website` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  `first` varchar(25) COLLATE utf8_bin DEFAULT NULL,
  `last` varchar(25) COLLATE utf8_bin DEFAULT NULL,
  `fbIDHASH` varchar(45) COLLATE utf8_bin DEFAULT NULL,
  `gender` int(2) DEFAULT NULL,
  `age` float DEFAULT NULL,
  `summary_file` varchar(255) COLLATE utf8_bin DEFAULT NULL,
  `year_of_birth` int(11) DEFAULT NULL,
  `month_of_birth` int(11) DEFAULT NULL,
  `deviceHASH` varchar(45) COLLATE utf8_bin DEFAULT NULL COMMENT 'DeviceID',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=158 DEFAULT CHARSET=utf8 COLLATE=utf8_bin$$

