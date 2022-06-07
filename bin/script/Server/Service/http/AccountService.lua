
AccountService = {}

local gateService
local messageComponent

function AccountService.Awake()
    gateService = App.GetComponent("GateService")
    messageComponent = App.GetComponent("MessageComponent")
    return messageComponent ~= nil and gateService ~= nil
end

function AccountService.Register(request)
    table.print(request)
    local requestInfo = Json.Decode(request.data)
    assert(requestInfo.account, "register account is nil")
    assert(requestInfo.password, "register password is nil")
    assert(requestInfo.phone_num, "register phone number is nil")
    return MysqlComponent.Add("db_account.tab_user_account", {
        user_id = Guid.Create(),
        register_time = os.time(),
        account = requestInfo.account,
        password = requestInfo.password,
        phone_num = requestInfo.phone_num,
    })
end

function AccountService.Login(request)
    local loginInfo = Json.Decode(request.data)
    assert(loginInfo, "request data error")
    assert(type(loginInfo.account) == "string", "user account is not string")
    assert(type(loginInfo.password) == "string", "user password is not string")
    local userInfo = MysqlComponent.QueryOnce("db_account.tab_user_account",{
        account = loginInfo.account
    })
    if loginInfo.password ~= userInfo.password then

    end
end