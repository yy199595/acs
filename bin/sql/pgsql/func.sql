PREPARE query_user_info AS SELECT * FROM user_info_list WHERE user_id=$1 LIMIT 1;
PREPARE query_user_simple_info AS SELECT (user_id,nick,icon) FROM user_info_list WHERE user_id=$1 LIMIT 1;
PREPARE insert_user_info AS INSERT INTO user_info_list(_id,nick,permission,user_desc,user_id,sex,public_id,icon,unionid,create_time)values($1,$2,$3,$4,$5,$6,$7,$8,$9,$10);