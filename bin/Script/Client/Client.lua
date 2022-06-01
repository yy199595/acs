
Client = {}
function Client.Awake()

    local msg = Message.New("c2s.GateAuth.Request", {
        token = "112233"
    })
    print(msg, type(msg))

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

    local authMessage = Message.New("c2s.GateAuth.Request", {
        token = loginInfo.token
    })

    clientComponent:Call("GateService.Auth", authMessage)

    local chatMessage = Message.New("c2s.Chat.Request",{
        msg_type = 1,  message = "nihaoa"
    })

    local code = clientComponent:Call("ChatService.Chat", "c2s.Chat.Request", chatMessage)
    Log.Error("code = ", code)
end