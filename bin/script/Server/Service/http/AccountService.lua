
local AccountService = {}
local tabName = "user.account_info"
local Mongo = require("Component.MongoComponent")
local RedisClient = require("Component.RedisComponent")


function AccountService.Start()
    print("启动账号服务")
    return true
end

function AccountService.Register(requestInfo, response)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")
    local userInfo = Mongo.QueryOnce(tabName, {
        _id = requestInfo.account
    })
    if userInfo ~= nil then
        response.error = "账号已经存在"
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
    Log.Info("register account : ", rapidjson.encode(requestInfo))
    return Mongo.InsertOnce(tabName, requestInfo, requestInfo.user_id)
end

function AccountService.Login(request, response)

    assert(type(request.account) == "string", "user account is not string")
    assert(type(request.password) == "string", "user password is not string")

    local userInfo = Mongo.QueryOnce(tabName, {
        _id = request.account
    })
    table.print(userInfo)
    if userInfo == nil or request.password ~= userInfo.password then
        response.error = "账号不存在或者密码错误"
        return XCode.Failure
    end
    local address = Service.AllotServer("OuterService")
    local code, data = Service.Call(address, "OuterService.Allot", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful or data == nil then
        response.error = "分配网关失败"
        return XCode.AllotUser
    end
    Mongo.Update(tabName, { _id = request.account },
                {  last_login_time = os.time(),  token = response.token }, userInfo.user_id)
    response.data = data
    return XCode.Successful
end

return AccountService