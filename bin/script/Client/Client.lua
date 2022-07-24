
Client = {}

function Client.Test(tab)
    print(Json.Encode(tab))
end

local account = "yjz1995"
local password = "123456"
local phoneNum = 13716061995


function Client.StartLogic()

    local clientComponent = App.GetComponent("ClientComponent")
    LoginComponent.Register(account, password, phoneNum)

    local loginInfo = LoginComponent.Login(account, password)
    if loginInfo.code ~= XCode.Successful then
        Log.Error("使用http登陆失败")
        return
    end

    local address = loginInfo.data.address
    if not clientComponent:StartConnectAsync(address) then
        Log.Error("连接网关服务器 [" , address, "] 失败")
        return
    end
    Log.Debug("连接网关服务器[" , address, "]成功")

    local code, _ = clientComponent:Call("GateService.Auth", "c2s.GateAuth.Request", {
        token = loginInfo.data.token
    })
    if code ~= XCode.Successful then
        Log.Error("user auth failure")
        return
    end
    coroutine.start(LoopCall)
    coroutine.start(LoopCall)
    coroutine.start(LoopCall)
    coroutine.start(LoopLogin)
    coroutine.start(LoopRegister)

end

function LoopLogin()
    while true do
        LoginComponent.Register(account, password, phoneNum)
    end
end

function LoopRegister()
    while true do
        LoginComponent.Login(account, password)
    end
end

function LoopCall()
    local clientComponent = App.GetComponent("ClientComponent")
    while true do
        local res, response = clientComponent:Call("ChatService.Chat", "c2s.Chat.Request", {
            user_id = 1122, msg_type = 1, message = "hello"
        })
        Log.Error("code = ", res, Json.Encode(response))
    end
end