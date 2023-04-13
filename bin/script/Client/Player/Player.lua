Player = {}
require("Client.LoginComponent")
function Player:Call(func, message)
    return Client.Call(self.session, func, message)
end

function Player:Send(func, message)
    Client.Send(self.session, func, message)
end

function Player:Login()
    local account = self.account
    local password = self.password
    local phoneNum = self.phone
    LoginComponent.Register(account, password, phoneNum)
    local loginInfo = LoginComponent.Login(account, password)
    if loginInfo == nil or loginInfo.code ~= XCode.Successful then
        Log.Error(account, ": 使用http登陆失败")
        table.print(loginInfo)
        return false
    end

    local token = loginInfo.data.token
    local address = loginInfo.data.address
    self.session = Client.New(loginInfo.data.address)
    local code = self:Call("Gate.Login", token)
    if code ~= XCode.Successful then
        Log.Error(account, " 登录网关服务器 [", address, "] 失败")
        return false
    end
    Log.Debug(account, " 登录网关服务器[", address, "]成功")
end

function Player.New(info)
    local tab = {
        account = info.account,
        password = info.passwd,
        phone = info.phoneNum,
        session = 0
    }
    setmetatable(tab, {__index = Player})
    return tab
end

function Player:Update()
    self:Login()
    while true do
        local code = self:Call("Chat.Chat", {
            user_id = 1122, msg_type = 1, message = "hello"
        })
        if code == XCode.NetWorkError then
            Log.Error("net work error stop call")
            return
        end
        --coroutine.sleep(1000)
    end
end
return Player