
AccountService = {}

local gateService
local redisComponent
local mysqlComponent
function AccountService.Awake()
    mysqlComponent = _G.MysqlComponent
    redisComponent = _G.RedisComponent
    gateService = App.GetComponent("GateService")
    return gateService ~= nil
end

function AccountService.Register(request)

    table.print(request)
    local requestInfo = Json.Decode(request.data)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")
    local user_id = redisComponent.AddCounter("main", "user_id")
    if user_id <= 0 then
        return XCode.Failure
    end
    return mysqlComponent.Add("account.user_info", {
        user_id = user_id,
        register_time = os.time(),
        account = requestInfo.account,
        password = requestInfo.password,
        phone_num = requestInfo.phone_num,
    })
end

function AccountService.Login(request)
    table.print(request)
    local loginInfo = Json.Decode(request.data)
    assert(loginInfo, "request data error")
    assert(type(loginInfo.account) == "string", "user account is not string")
    assert(type(loginInfo.password) == "string", "user password is not string")
    local userInfo = mysqlComponent.QueryOnce("account.user_info",{
        account = loginInfo.account
    })
    Log.Warning(userInfo)
    if loginInfo.password ~= userInfo.password then
        return XCode.Failure
    end
    local address = gateService:GetAddress()
    local code, response = gateService:Call(address, "AllotUser", {
        value = userInfo.user_id
    })
    if code ~= XCode.Successful then
        return XCode.AllotUser
    end
    local ip, _ = StringUtil.ParseAddress(request.address)
    mysqlComponent.Update("account.user_info", { account = loginInfo.account },
            {  last_login_time = os.time(), last_login_ip = ip,  token = response.token })
    return XCode.Successful, response
end