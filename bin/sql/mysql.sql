CREATE TABLE IF NOT EXISTS user_list
(
    user_id BIGINT       NOT NULL DEFAULT 0 UNIQUE COMMENT '用户ID', /*唯一ID*/
    open_id VARCHAR(64)  NOT NULL DEFAULT '' UNIQUE COMMENT '微信返回的opendid',
    nick    VARCHAR(64)  NOT NULL DEFAULT '' COMMENT '昵称',
    city    BIGINT       NOT NULL DEFAULT 0 COMMENT '城市',
    icon    VARCHAR(128) NOT NULL DEFAULT '' COMMENT '用户头像',
    sex     INT          NOT NULL DEFAULT 0 COMMENT '用户性别',
    level   INT          NOT NULL DEFAULT 1 COMMENT '用户等级',
    attr    JSON         COMMENT '用户属性',
    intro   VARCHAR(128) DEFAULT '' COMMENT '用户简介',
    PRIMARY KEY (user_id)
) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;

