
require "Util.TxtUtil"
require "Util.JsonUtil"
local clientManager = nil
local loginSrvSession = nil

local LoginManager = {}

function LoginManager.OnRegisterBack(gameObject, code, messageData)
    local str = JsonUtil.Encode(messageData)
    print(gameObject, code, str)

end


function LoginManager.OnLoginBack(gameObject, code, messageData)


end

function LoginManager.OnConnectLoginSrv(gameObject, isLinkSuccess)

    loginSrvSession = gameObject:AddComponent("RobotNetController")

    if loginSrvSession == nil then
        return
    end

    local registerData = {}
    registerData.mAccount = "646585122@qq.com"
    registerData.mPassWord = "646585122@QQ.COM"
    loginSrvSession:CallSrvFunction("LoginSrv", "LoginMgr::OnRegister", "SayNoProtoc.RegisterData", registerData)
end


function main()
    
    clientManager = frameWork:GetManager("NetMessageManager")
    if clientManager ~= nil then

        clientManager:Bind("LoginManager.OnLoginBack", "SayNoProtoc.LoginData", LoginManager.OnLoginBack)
        clientManager:Bind("LoginManager.OnRegisterBack", "SayNoProtoc.LoginData", LoginManager.OnRegisterBack)


        local tab = TxtUtil.ReadJsonFile("./Config/RobotConfig.json")
        print(tab.ip, tab.port)
        local config = SayNoSrvConfig.New(tab.ip, tab.port, "LoginSrv", 1)
        clientManager:ConnectLogicSrv(config, LoginManager.OnConnectLoginSrv)
    end
end

