
OrderType = {
    VipOrder = 1,  --vip订单
    ActivityOrder = 2 --活动订单
}

OrderStatus = {
    None = 0, --未支付
    Timeout = 1, --超时
    Done = 2, --成功
    WaitRefund = 3, --等待退款
    Refund = 4 --退款
}

ActivityStatus = {
    Signing = 0, --正在报名
    UnderWay = 1, --进行中
    AlreadyOver = 2, --已经结束
    Dissolve = 4,
}

MongoTab = {
    USER_INFO_LIST = "user_info_list"
}