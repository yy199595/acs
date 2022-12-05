
local AccountService = {}
local tabName = "user.account_info"
local Mongo = LuaRequire("Component.MongoComponent")
local RedisClient = LuaRequire("Component.RedisComponent")


function AccountService.Start()
    print("启动账号服务")
    return true
end

function AccountService.Register(requestInfo)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")
    local userInfo = Mongo.QueryOnce(tabName, {
        _id = requestInfo.account
    })
    if userInfo ~= nil then
        return XCode.AccountAlreadyExists
    end
    local nowTime = os.time()
    local user_id = RedisClient.AddCounter("user_id")
    local str = string.format("%s%d%d", requestInfo.address, nowTime, user_id)

    requestInfo.login_time = 0
    requestInfo.user_id = user_id
    requestInfo.create_time = nowTime
    requestInfo._id = requestInfo.account
    requestInfo.token = Md5.ToString(str)
    Log.Info("register account : ", Json.Encode(requestInfo))
    return Mongo.Update(tabName, {
        _id = requestInfo.account
    }, requestInfo)
end

function AccountService.Login(request)

    assert(type(request.account) == "string", "user account is not string")
    assert(type(request.password) == "string", "user password is not string")

    local userInfo = Mongo.QueryOnce(tabName, {
        _id = request.account
    })
    table.print(userInfo)
    if userInfo == nil or request.password ~= userInfo.password then
        assert(false, request.account .. " : 账号不存在或者密码错误")
        return XCode.Failure
    end
    local address = Service.AllotServer("OuterService")
    local code, response = Service.Call(address, "OuterService.Allot", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful then
        return XCode.AllotUser
    end
    Mongo.Update(tabName, { _id = request.account },
                {  last_login_time = os.time(),  token = response.token })
    return XCode.Successful, response
end

return AccountService