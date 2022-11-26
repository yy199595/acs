
local AccountService = {}
function AccountService.Start111()
    print("启动账号服务")
    return true
end

function AccountService.Update(tick)
    Log.Info("tick = ", tick)
end

function AccountService.Register(requestInfo)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")

    local userInfo = DataMgrComponent.Get("user.account", requestInfo.account)

    if userInfo ~= nil then
        return XCode.AccountAlreadyExists
    end
    local nowTime = os.time()
    local user_id = RedisComponent.AddCounter("user_id")
    local str = string.format("%s%d%d", request.address, nowTime, user_id)

    requestInfo.login_time = 0
    requestInfo.user_id = user_id
    requestInfo.create_time = nowTime
    requestInfo._id = requestInfo.account
    requestInfo.token = Md5.ToString(str)
    requestInfo.address = StringUtil.ParseAddress(request.address)
    DataMgrComponent.Set("user.account", requestInfo.account, requestInfo, true)
    return XCode.Successful
end

function AccountService.Login(request)

    table.print(request)
    assert(type(request.account) == "string", "user account is not string")
    assert(type(request.password) == "string", "user password is not string")

    local userInfo = DataMgrComponent.Get("user.account",request.account)

    if userInfo == nil or request.password ~= userInfo.password then
        return XCode.Failure
    end
    local address = Service.AllotLocation("OuterService")
    local code, response = Service.Call(address, "OuterService.Allot", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful then
        return XCode.AllotUser
    end
    table.print(response)
    MongoComponent.Update("user.account", { _id = request.account },
                {  last_login_time = os.time(),  token = response.token })
    return XCode.Successful, response
end

return AccountService