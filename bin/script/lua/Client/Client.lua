
Client = {}

function Client.Test(tab)
    print(Json.Encode(tab))
end

local account = "yjz1995"
local password = "123456"
local phoneNum = 13716061995
local clientComponent = App.GetComponent("ClientComponent")

function Client.Start()

    local loginComponent = require("Component.LoginComponent")
    loginComponent.Awake()
    loginComponent.Register(account, password, phoneNum)

    local loginInfo = loginComponent.Login(account, password)
    if loginInfo == nil or loginInfo.code ~= XCode.Successful then
        Log.Error("使用http登陆失败")
        return false
    end

    table.print(loginInfo)
    local address = loginInfo.data.address
    if not clientComponent:StartConnectAsync(address) then
        Log.Error("连接网关服务器 [" , address, "] 失败")
        return false
    end
    Log.Debug("连接网关服务器[" , address, "]成功")



    local code, _ = clientComponent:Auth(loginInfo.data.token)
    if code ~= XCode.Successful then
        Log.Error("user auth failure")
        return false
    end
    --coroutine.start(LoopCall)
    return true
end
local callCount = 0


function LoopCall()
    while true do
        local t1 = Time.NowMilTime()
        local res, response = clientComponent:Call("ChatService.Chat", "c2s.chat.request", {
            user_id = 1122, msg_type = 1, message = "hello"
        })
        callCount = callCount + 1
        local t = Time.NowMilTime() - t1
        Log.Error(string.format("call use time = [%dms] count = %d", t, callCount))
    end
end

return Client