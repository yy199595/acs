
Client = {}

function Client.Test(tab)
    print(Json.Encode(tab))
end

local account = "yjz1995"
local password = "123456"
local phoneNum = 13716061995
local clientComponent = App.GetComponent("ClientComponent")

function Client.Start()

    LoginComponent.Awake()
    LoginComponent.Register(account, password, phoneNum)

    local loginInfo = LoginComponent.Login(account, password)
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
    coroutine.start(LoopPing)
    return true
end
local callCount = 0
local loginCount = 0
local registerCount = 0

function LoopPing()
    while true do
        local t1 = Time.GetNowMilTime()
        local code = clientComponent:Call("GateService.Ping")
        Log.Info("ping use time ", Time.GetNowMilTime() - t1)
    end
end

function LoopRegister()
    while true do
        local t1 = Time.GetNowMilTime()
        local account1 = string.format("%d@qq.com", 1000 + registerCount)
        LoginComponent.Register(account1, password, phoneNum)
        registerCount = registerCount + 1
        Log.Warning(string.format("register use time = [%dms] count = %d", Time.GetNowMilTime() - t1, registerCount))
    end
end

function LoopLogin()
    while true do
        local t1 = Time.GetNowMilTime()
        local account1 = string.format("%d@qq.com", 1000 + loginCount)

        LoginComponent.Login(account1, password)
        loginCount = loginCount + 1
        local t = Time.GetNowMilTime() - t1
        Log.Info(string.format("login use time = [%dms] count = %d", t, loginCount))
    end
end

function LoopCall()
    while true do
        local t1 = Time.GetNowMilTime()
        local res, response = clientComponent:Call("ChatService.Chat", "c2s.chat.request", {
            user_id = 1122, msg_type = 1, message = "hello"
        })
        callCount = callCount + 1
        local t = Time.GetNowMilTime() - t1
        Log.Error(string.format("call use time = [%dms] count = %d", t, callCount))
    end
end

return Client