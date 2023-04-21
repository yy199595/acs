
local AccountService = {}
local tabName = "user.account_info"
local mysql = require("Server.MysqlClient")
local redis = require("Server.RedisComponent")
function AccountService.Awake()
    print("启动账号服务")
    return mysql.NewTable(0, tabName, {
        pb = tabName,
        keys = { "account "}
    })
end

function AccountService.Register(request)

    table.print(request)
    local message = rapidjson.decode(request.message)
    assert(message.account, "register account is nil")
    assert(message.password, "register password is nil")
    assert(message.phone_num, "register phone number is nil")

    local id = mysql.Open()
    local result = mysql.QueryOne(id, tabName, {
        "user_id"
    }, { account = message.account })
    if result ~= nil then
        return XCode.AccountAlreadyExists
    end

    local nowTime = os.time()
    local userId = redis.AddCounter("user_id")
    local str = string.format("%s%d%d", ip, nowTime, userId)
    local ip = string.match(request.from, "http://(%d+.%d+.%d+.%d+):%d+")
    local data = {
        user_id = userId,
        login_time = nowTime,
        create_time = nowTime,
        account = message.account,
        password = message.password,
        phone_num = message.phone_num,
        last_login_ip = ip,
        token = Md5.ToString(str)
    }
    result = mysql.Insert(id, tabName, data)
    if not result then
        return XCode.SaveToMysqlFailure
    end
    Log.Info("register account : ", rapidjson.encode(message))
    return XCode.Successful
end

function AccountService.Login(request)

    assert(type(request.account) == "string", "user account is not string")
    assert(type(request.password) == "string", "user password is not string")

    local userInfo = Mongo.QueryOnce(tabName, {
        _id = request.account
    })
    if userInfo == nil or request.password ~= userInfo.password then
        return XCode.Failure, "账号不存在或者密码错误"
    end

    local code, data = Service.Call(userInfo.user_id, "Gate.Allocation", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful or data == nil then
        return XCode.AllotUser, "分配网关失败"
    end
    Mongo.Update(tabName, { _id = request.account },
                {  last_login_time = os.time(),  token = data.token }, userInfo.user_id)
    Log.Warn(string.format("玩家%s登录成功,玩家id=%d", request.account, userInfo.user_id))
    return XCode.Successful, data
end

return AccountService