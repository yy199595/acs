
local AccountService = {}
local tabName = "user.account_info"
local Mongo = require"Server.MongoComponent"

function AccountService.Awake()
    print("启动账号服务")
    return true
end

function AccountService.Register(request)
    assert(request.account, "register account is nil")
    assert(request.password, "register password is nil")
    assert(request.phone_num, "register phone number is nil")

    local account = request.account
    local userInfo = Mongo.QueryOnce(tabName, {
        _id = request.account
    })
    if userInfo ~= nil then
        return XCode.AccountAlreadyExists, "账号已经存在"
    end
    local nowTime = os.time()
    local user_id = Guid.Create()
    Log.Info(string.format("start register account %s", account))
    local str = string.format("%s%d%d", request.address, nowTime, user_id)

    request.user_id = user_id
    request.login_time = nowTime
    request.create_time = nowTime
    request._id = requestInfo.account
    request.token = Md5.ToString(str)
    Log.Info("register account : ", rapidjson.encode(request))
    if Mongo.InsertOnce(tabName, request, request.user_id) == XCode.Successful then
        Log.Warn("注册账号", account, "成功, 玩家id=", user_id)
        return XCode.Successful
    end
    Log.Error("保存数据到mongodb失败,注册失败")
    return XCode.Failure, "保存数据到mongodb失败"
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