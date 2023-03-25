
local AccountService = {}
local tabName = "user.account_info"
local Mongo = require "MongoComponent"
local RedisClient = require "RedisComponent"


function AccountService.Start()
    print("启动账号服务")
    return true
end

function AccountService.Register(requestInfo, response)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")

    local account = requestInfo.account
    local userInfo = Mongo.QueryOnce(tabName, {
        _id = requestInfo.account
    })
    if userInfo ~= nil then
        response.error = "账号已经存在"
        RedisClient.Run("HSET", "user", account, response)
        return XCode.AccountAlreadyExists
    end
    local nowTime = os.time()
    local user_id = Guid.Create()
    Log.Info(string.format("start register account %s", account))
    local str = string.format("%s%d%d", requestInfo.address, nowTime, user_id)

    requestInfo.login_time = 0
    requestInfo.user_id = user_id
    requestInfo.create_time = nowTime
    requestInfo._id = requestInfo.account
    requestInfo.token = Md5.ToString(str)
    RedisClient.Run("HSET", "user", account, requestInfo)
    Log.Info("register account : ", rapidjson.encode(requestInfo))
    if Mongo.InsertOnce(tabName, requestInfo, requestInfo.user_id) == XCode.Successful then
        Log.Warn("注册账号", account, "成功, 玩家id=", user_id)
        return XCode.Successful
    end
    response.error = "保存数据到mongodb失败"
    Log.Error("保存数据到mongodb失败，注册失败")
    return XCode.Failure
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