
Client = {}
function Client.Awake()
    LoginComponent.Awake()
end

local account = "yjz1995"
local password = "123456"
local phoneNum = 13716061995

function Client.Start()

    local clientComponent = App.GetComponent("ClientComponent")

    LoginComponent.Register(account, password, phoneNum)

    local loginInfo = LoginComponent.Login(account, password)

    if not clientComponent:StartConnect(loginInfo.address) then
        Log.Error("connect [" , loginInfo, "] failure")
        return
    end
    Log.Debug("connect [" , loginInfo.address, "] successful")

    clientComponent:Call("GateService.Auth", "c2s.GateAuth.Request", {
        token = loginInfo.token
    })

    local code = clientComponent:Call("ChatService.Chat", "c2s.Chat.Request", {
        msg_type = 1,  message = "nihaoa"
    })
    Log.Error("code = ", code)
end