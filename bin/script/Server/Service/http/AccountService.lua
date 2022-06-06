
AccountService = {}
local mysqlComponent
local messageComponent
function AccountService.Awake()
    messageComponent = App.GetComponent("MessageComponent")
    mysqlComponent = App.GetComponent("MysqlAgentComponent")
    return true
end

function AccountService.Register(request)
    local requestInfo = Json.Decode(request.data)
    local registerInfo = messageComponent:New("db_account.tab_user_account",{
        user_id = 123456,
        account = requestInfo.account,
        password = requestInfo.password,
        phone_num = requestInfo.phone_num
    })
    local code = mysqlComponent:Add(registerInfo)
end

function AccountService.Login(request)

end