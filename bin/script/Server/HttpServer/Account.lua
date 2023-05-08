require("XCode")
local log = require("Log")
local AccountService = {}
local tabName = "user.account_info"
local mysql = require("Server.MysqlClient")
local redis = require("Server.RedisComponent")
function AccountService.Awake()
    print("启动账号服务")
    Proto.Import("mysql/user.proto")
    return mysql.NewTable(0, tabName, {
        pb = tabName,
        keys = { "account" }
    })
end

function AccountService.Register(request)

    table.print(request)
    local message = request.message
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
    local ip = string.match(request.from, "http://(%d+.%d+.%d+.%d+):%d+")
    local str = string.format("%s%d%d", ip, nowTime, userId)
    local data = {
        user_id = userId,
        login_time = nowTime,
        register_time = nowTime,
        account = message.account,
        pass_word = message.password,
        phone_num = message.phone_num,
        last_login_ip = ip,
        login_token = Md5.ToString(str)
    }
    result = mysql.Insert(id, tabName, data)
    if not result then
        return XCode.SaveToMysqlFailure
    end
    log.Info("register account : ", message)
    return XCode.Successful
end

function AccountService.Login(request)

    local message = request.message
    assert(type(message.account) == "string", "user account is not string")
    assert(type(message.password) == "string", "user password is not string")

    local id = mysql.Open()
    local userInfo = mysql.QueryOne(id, tabName, { "pass_word", "user_id" }, {
        account = message.account
    })

    if userInfo == nil or request.password ~= userInfo.password then
        return XCode.Failure, "账号不存在或者密码错误"
    end
    local ip = string.match(request.from, "http://(%d+.%d+.%d+.%d+):%d+")
    local str = string.format("%s%d%d", ip, nowTime, userId)
    local token = Md5.ToString(str)

    local serverId = Service.RangeServer("Gate")
    local rpcAddress = Service.GetAddrById(serverId, "rpc")
    local gateAddress = Service.GetAddrById(serverId, "gate")

    local code = Service.Call(rpcAddress, "Gate.Allocation", {
        token = token,
        user_id = userInfo.user_id
    })

    if code ~= XCode.Successful then
        return XCode.AddressAllotFailure, "分配网关失败"
    end

    local data = {
        last_login_ip = ip,
        login_time = os.time(),
        login_token = token
    }
    local res = mysql.Update(id, tabName, data, {
        account = message.account
    })
    if not res then
        return XCode.SaveToMysqlFailure, "更新数据失败"
    end
    log.Warn(string.format("玩家%s登录成功,玩家id=%d", request.account, userInfo.user_id))
    return XCode.Successful, {
        token = token,
        address = gateAddress
    }
end

return AccountService