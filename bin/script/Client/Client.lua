
Client = {}

function Client.Test(tab)
    print(Json.Encode(tab))
end

local account = "yjz1995"
local password = "123456"
local phoneNum = 13716061995


function Client.StartLogic()

    local clientComponent = App.GetComponent("ClientComponent")
    local messageComponent = App.GetComponent("MessageComponent")

    LoginComponent.Register(account, password, phoneNum)

    local loginInfo = LoginComponent.Login(account, password)
    table.print(loginInfo)
    if loginInfo.code ~= XCode.Successful then
        return
    end
    local address = loginInfo.data.address
    if not clientComponent:StartConnectAsync(address) then
        Log.Error("connect [" , address, "] failure")
        return
    end
    Log.Debug("connect h t t[" , address, "] successful")

    local authMessage = messageComponent:New("c2s.GateAuth.Request", {
        token = loginInfo.data.token
    })

    clientComponent:Call("GateService.Auth", authMessage)


    --while true do
    --
    --    local testtMessage = messageComponent:New("c2s.Chat.Request",{
    --        user_id = 1122, msg_type = 1, message = "hello"
    --    })
    --
    --    local code, res = clientComponent:Call("ChatService.Chat", testtMessage)
    --    Log.Error("code = ", code, Json.Encode(res))
    --end
end