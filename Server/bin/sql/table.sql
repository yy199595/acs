
DROP TABLE IF EXISTS `tb_player_account`;
	CREATE TABLE `tb_player_account` (
	`account` varchar(64) NOT NULL DEFAULT '0' COMMENT '玩家账户',
	`areaud` int(20) NOT NULL DEFAULT 0 COMMENT '区服id',
	`userid` bigint(20) NOT NULL DEFAULT 0 COMMENT '用户id',
	`passwd` varchar(32) NOT NULL DEFAULT '0' COMMENT '账户密码',
	`registertime` bigint(20) NOT NULL DEFAULT 0 COMMENT '注册时间',
	`lastlogintime` bigint(20) NOT NULL DEFAULT 0 COMMENT '上次登录时间',
	`lastloginip` varchar(32) NOT NULL DEFAULT '0' COMMENT '上次登录ip',
	 `token` varchar(64) NOT NULL DEFAULT '0' COMMENT '登录token',
	PRIMARY KEY (`account`, `userid`)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='玩家账户数据';