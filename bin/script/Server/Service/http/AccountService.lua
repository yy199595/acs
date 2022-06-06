
AccountService = {}

local messageComponent
function AccountService.Awake()
    messageComponent = App.GetComponent("MessageComponent")
    return messageComponent ~= nil
end

function AccountService.Register(request)
    table.print(request)
    local requestInfo = Json.Decode(request.data)
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
    local userInfo = MysqlComponent.QueryOnce("db_account.tab_user_account",{
        account = loginInfo.account
    })
    table.print(loginInfo)
end