
local AccountService = {}
local Mongo = require("Component.MongoComponent")
local DataMgr = require("Component.DataMgrComponent")

function AccountService.Start()
    print("启动账号服务")
    return true
end

function AccountService.Register(requestInfo)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")

    local userInfo = DataMgr.Get("user.account", requestInfo.account)

    if userInfo ~= nil then
        return XCode.AccountAlreadyExists
    end
    local nowTime = os.time()
    local user_id = LuaRedisComponent.AddCounter("user_id")
    local str = string.format("%s%d%d", requestInfo.address, nowTime, user_id)

    requestInfo.login_time = 0
    requestInfo.user_id = user_id
    requestInfo.create_time = nowTime
    requestInfo._id = requestInfo.account
    requestInfo.token = Md5.ToString(str)
    DataMgr.Set("user.account", requestInfo.account, requestInfo, true)
    return XCode.Successful
end

function AccountService.Login(request)

    --table.print(request)
    assert(type(request.account) == "string", "user account is not string")
    assert(type(request.password) == "string", "user password is not string")

    local userInfo = DataMgr.Get("user.account",request.account)
    if userInfo == nil or request.password ~= userInfo.password then
        return XCode.Failure
    end
    local address = Service.AllotServer("OuterService")
    local code, response = Service.Call(address, "OuterService.Allot", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful then
        return XCode.AllotUser
    end
    table.print(response)
    Mongo.Update("user.account", { _id = request.account },
                {  last_login_time = os.time(),  token = response.token })
    return XCode.Successful, response
end

return AccountService