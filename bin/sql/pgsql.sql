CREATE TABLE IF NOT EXISTS user_list
(
    user_id BIGINT       NOT NULL DEFAULT 0 UNIQUE,
    open_id VARCHAR(64)  NOT NULL DEFAULT '' UNIQUE,
    nick    VARCHAR(64)  NOT NULL DEFAULT '',
    city    BIGINT       NOT NULL DEFAULT 0,
    icon    VARCHAR(128) NOT NULL DEFAULT '',
    sex     INTEGER      NOT NULL DEFAULT 0,
    level   INTEGER      NOT NULL DEFAULT 1,
    attr    JSON,
    intro   VARCHAR(128)          DEFAULT '',
    PRIMARY KEY (user_id)
);

COMMENT ON TABLE user_list IS '用户表';
COMMENT ON COLUMN user_list.user_id IS '用户ID';
COMMENT ON COLUMN user_list.open_id IS '微信返回的opendid';
COMMENT ON COLUMN user_list.nick IS '昵称';
COMMENT ON COLUMN user_list.city IS '城市';
COMMENT ON COLUMN user_list.icon IS '用户头像';
COMMENT ON COLUMN user_list.sex IS '用户性别';
COMMENT ON COLUMN user_list.level IS '用户等级';
COMMENT ON COLUMN user_list.attr IS '用户属性';
COMMENT ON COLUMN user_list.intro IS '用户简介';