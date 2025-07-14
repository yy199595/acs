CREATE TABLE IF NOT EXISTS yjz.activity_data_list
(
    refund     BIGINT      NOT NULL DEFAULT 0,
    amount     BIGINT      NOT NULL DEFAULT 0,
    commission BIGINT      NOT NULL DEFAULT 0,
    sub_amount BIGINT      NOT NULL DEFAULT 0,
    _id        BIGINT      NOT NULL DEFAULT 0,
    share      BIGINT      NOT NULL DEFAULT 0,
    browse     BIGINT      NOT NULL DEFAULT 0,.
    city       BIGINT      NOT NULL DEFAULT 0,
    title      VARCHAR(64) NOT NULL DEFAULT '',
    sign_up    BIGINT      NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS yjz.activity_list
(
    _id         BIGINT      NOT NULL DEFAULT 0,
    tag         VARCHAR(64) NOT NULL DEFAULT '',
    city        BIGINT      NOT NULL DEFAULT 0,
    type        BIGINT      NOT NULL DEFAULT 0,
    price       BIGINT      NOT NULL DEFAULT 0,
    status      BIGINT      NOT NULL DEFAULT 0,
    club_id     BIGINT      NOT NULL DEFAULT 0,
    content     VARCHAR(64) NOT NULL DEFAULT '',
    open_id     BIGINT      NOT NULL DEFAULT 0,
    reviewed_id BIGINT      NOT NULL DEFAULT 0,
    vip_price   BIGINT      NOT NULL DEFAULT 0,
    dou_price   BIGINT      NOT NULL DEFAULT 0,
    drive_price BIGINT      NOT NULL DEFAULT 0,
    latitude    VARCHAR(64) NOT NULL DEFAULT '',
    longitude   VARCHAR(64) NOT NULL DEFAULT '',
    collect     VARCHAR(64) NOT NULL DEFAULT '',
    show_user   BOOLEAN     NOT NULL DEFAULT 0,
    target_id   BIGINT      NOT NULL DEFAULT 0,
    commission  BIGINT      NOT NULL DEFAULT 0,
    code        VARCHAR(64) NOT NULL DEFAULT '',
    phone       VARCHAR(64) NOT NULL DEFAULT '',
    url         VARCHAR(64) NOT NULL DEFAULT '',
    title       VARCHAR(64) NOT NULL DEFAULT '',
    max_num     BIGINT      NOT NULL DEFAULT 0,
    address     VARCHAR(64) NOT NULL DEFAULT '',
    stop_time   BIGINT      NOT NULL DEFAULT 0,
    start_time  BIGINT      NOT NULL DEFAULT 0,
    create_time BIGINT      NOT NULL DEFAULT 0,
    sign_time   BIGINT      NOT NULL DEFAULT 0,
    refund_time BIGINT      NOT NULL DEFAULT 0,
    users       JSON
);

CREATE TABLE IF NOT EXISTS yjz.club_list
(
    members     JSON,
    _id         BIGINT      NOT NULL DEFAULT 0,
    nick        VARCHAR(64) NOT NULL DEFAULT '',
    city        BIGINT      NOT NULL DEFAULT 0,
    icon        VARCHAR(64) NOT NULL DEFAULT '',
    desc        VARCHAR(64) NOT NULL DEFAULT '',
    user_id     BIGINT      NOT NULL DEFAULT 0,
    address     VARCHAR(64) NOT NULL DEFAULT '',
    latitude    VARCHAR(64) NOT NULL DEFAULT '',
    longitude   VARCHAR(64) NOT NULL DEFAULT '',
    invite_id   BIGINT      NOT NULL DEFAULT 0,
    create_time BIGINT      NOT NULL DEFAULT 0,
    act_count   BIGINT      NOT NULL DEFAULT 0,
    num_count   BIGINT      NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS yjz.counter
(
    _id   VARCHAR(64) NOT NULL DEFAULT '',
    field VARCHAR (64) NOT NULL DEFAULT '',
    value BIGINT      NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS yjz.order_list
(
    _id          VARCHAR(64) NOT NULL DEFAULT '',
    city         BIGINT      NOT NULL DEFAULT 0,
    club_id      BIGINT      NOT NULL DEFAULT 0,
    target_id    BIGINT      NOT NULL DEFAULT 0,
    commission   BIGINT      NOT NULL DEFAULT 0,
    inviter_id   BIGINT      NOT NULL DEFAULT 0,
    desc         VARCHAR(64) NOT NULL DEFAULT '',
    type         BIGINT      NOT NULL DEFAULT 0,
    price        BIGINT      NOT NULL DEFAULT 0,
    price_type   BIGINT      NOT NULL DEFAULT 0,
    amount       BIGINT      NOT NULL DEFAULT 0,
    status       BIGINT      NOT NULL DEFAULT 0,
    user_id      BIGINT      NOT NULL DEFAULT 0,
    open_id      VARCHAR(64) NOT NULL DEFAULT '',
    icon         VARCHAR(64) NOT NULL DEFAULT '',
    over_time    BIGINT      NOT NULL DEFAULT 0,
    refund_time  BIGINT      NOT NULL DEFAULT 0,
    product_id   BIGINT      NOT NULL DEFAULT 0,
    prepay_id    VARCHAR(64) NOT NULL DEFAULT '',
    create_time  BIGINT      NOT NULL DEFAULT 0,
    custom       VARCHAR(64) NOT NULL DEFAULT '',
    refund_price BIGINT      NOT NULL DEFAULT 0,
    sallet_time  BIGINT      NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS yjz.user_info_list
(
    tag           VARCHAR(64) NOT NULL DEFAULT '',
    score         BIGINT      NOT NULL DEFAULT 0,
    members       JSON,
    login_type    VARCHAR(64) NOT NULL DEFAULT '',
    birthday      BIGINT      NOT NULL DEFAULT 0,
    _id           VARCHAR(64) NOT NULL DEFAULT '',
    sex           BIGINT      NOT NULL DEFAULT 0,
    descr          VARCHAR(64) NOT NULL DEFAULT '',
    nick          VARCHAR(64) NOT NULL DEFAULT '',
    icon          VARCHAR(64) NOT NULL DEFAULT '',
    user_id       BIGINT      NOT NULL DEFAULT 0,
    public_id     VARCHAR(64) NOT NULL DEFAULT '',
    city          BIGINT      NOT NULL DEFAULT 0,
    amount        BIGINT      NOT NULL DEFAULT 0,
    club_id       BIGINT      NOT NULL DEFAULT 0,
    card_id       BIGINT      NOT NULL DEFAULT 0,
    unionid       VARCHAR(64) NOT NULL DEFAULT '',
    city_name     VARCHAR(64) NOT NULL DEFAULT '',
    permission    BIGINT      NOT NULL DEFAULT 0,
    vip_time      BIGINT      NOT NULL DEFAULT 0,
    create_time   BIGINT      NOT NULL DEFAULT 0,
    activity_list JSON
);

CREATE TABLE IF NOT EXISTS yjz.user_wallet
(
    _id        VARCHAR(64) NOT NULL DEFAULT '',
    time       BIGINT      NOT NULL DEFAULT 0,
    list       JSON,
    amount     BIGINT      NOT NULL DEFAULT 0,
    reason     VARCHAR(64) NOT NULL DEFAULT '',
    user_id    BIGINT      NOT NULL DEFAULT 0,
    product_id BIGINT      NOT NULL DEFAULT 0
);