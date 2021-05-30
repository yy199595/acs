
ClientManager = { }
local this = ClientManager

function ClientManager.OnConnectComplate(tcpSession)

    SoEasy.Error(tcpSession, SoEasy.CallBySession)
    local cor = coroutine.create(this.Start)
    coroutine.resume(cor, tcpSession)
end

function ClientManager.Start(tcpSession)

    local registerData = { }
    registerData.account = "6465851222@qq.com"
    registerData.password = "199595yjz."
    registerData.phonenum = 13716061995
    registerData.platform = "ios_qq"
    registerData.device_mac = "0xs1dsx"

    local message = SoEasy.CreateByTable("PB.UserRegisterData", registerData)
    local code = SoEasy.CallBySession(tcpSession, "LoginService", "Register", message)
    print("register back", code)

    local loginData = { }
    loginData.account = registerData.account
    loginData.passwd = registerData.password
    local loginMessage = SoEasy.CreateByTable("PB.UserAccountData", loginData)
    if loginMessage ~= nil then
        local code = SoEasy.CallBySession(tcpSession, "LoginService", "Login", loginMessage)
        print("login back", code)
    end
end


return ClientManager