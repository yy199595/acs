
DROP TABLE IF EXISTS `tb_player_account`;
	CREATE TABLE `tb_player_account` (
	`account` varchar(64) COMMENT '玩家账户',
	`platform` varchar(64)  COMMENT '平台',
	`userid` bigint(20) COMMENT '用户id',
	`passwd` varchar(32) COMMENT '账户密码',
	`phonenum` bigint(20) NOT NULL DEFAULT 0 COMMENT '手机号',
	`registertime` varchar(64) COMMENT '注册时间',
	PRIMARY KEY (`account`, `userid`)
	) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='玩家账户数据';