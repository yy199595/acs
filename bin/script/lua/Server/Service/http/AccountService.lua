
AccountService = {}

local outerService = App.GetComponent("OuterService")

function AccountService.OnServiceStart()
    print("启动账号服务")
end

function AccountService.Register(request)
    local requestInfo = Json.Decode(request.data)
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

    local loginInfo = Json.Decode(request.data)
    assert(loginInfo, "request data error")
    assert(type(loginInfo.account) == "string", "user account is not string")
    assert(type(loginInfo.password) == "string", "user password is not string")

    local userInfo = DataMgrComponent.Get("user.account",loginInfo.account)

    if userInfo == nil or loginInfo.password ~= userInfo.password then
        return XCode.Failure
    end
    local address = outerService:AllotLocation()
    local code, response = outerService:Call(address, "AllotUser", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful then
        return XCode.AllotUser
    end
    table.print(response)
    local ip, _ = StringUtil.ParseAddress(request.address)
    MongoComponent.Update("user.account", { _id = loginInfo.account },
                {  last_login_time = os.time(), last_login_ip = ip,  token = response.token })
    return XCode.Successful, response
end

return AccountService