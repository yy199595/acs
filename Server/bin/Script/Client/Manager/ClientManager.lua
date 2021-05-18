
ClientManager = { }
local this = ClientManager

function ClientManager.OnConnectComplate(tcpSession)

    local cor = coroutine.create(this.Start)
    coroutine.resume(cor, tcpSession)
end

function ClientManager.Start(tcpSession)

    SoEasy.Sleep(3000)
    local registerData = { }
    registerData.account = "6465851222@qq.com"
    registerData.password = "199595yjz."
    registerData.phonenum = 13716061995
    registerData.platform = "ios_qq"
    registerData.device_mac = "0xs1dsx"

    local message = SoEasy.CreateByTable("PB.UserRegisterData", registerData)
    local code = SoEasy.CallBySession(tcpSession, "LoginManager.Register", message)
    print(code)
end


return ClientManager