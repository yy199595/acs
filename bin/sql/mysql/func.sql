SET GLOBAL long_query_time=2; #设置慢查询时间
PREPARE query_user_info FROM 'SELECT * FROM user_info_list WHERE user_id=?';
PREPARE query_simple_info FROM 'SELECT user_id,nick,amount,icon FROM user_info_list WHERE user_id=?';