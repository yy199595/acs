
local Log = require("Log")
local json = require("json")
local http = require("Http")
local LoginComponent = Class()

local host = "http://127.0.0.1:80/%s"

function LoginComponent:Start()
    local passwd = self.player.passwd
    local account = self.player.account

    self:Register(account, passwd)
    local code, response = self:Login(account, passwd)
    if code == XCode.Successful then
        Log.Info(self.player.account, " login successful")
    else
        Log.Error(self.player.account, " login failure")
        return
    end
    self.player:_Invoke("OnLogin", response)
end

function LoginComponent:Register(account, passwd)
    local url = string.format(host, "user/register")
    local response = http.Post(url, {
        account = account,
        password = passwd,
    })
    if response.code ~= 200 then
        Log.Error(response.status)
        return nil
    end
    local message = json.decode(response.body)
    if message.code ~= XCode.Successful then
        return nil
    end
    return message.code, message.data
end

function LoginComponent:Login(account, passwd) -- 获取gate地址
    local url = string.format(host, "user/login")
    local response = http.Post(url, {
        account = account,
        password = passwd
    })
    if response.code ~= 200 then
        Log.Error(response.status)
        return { code = XCode.Failure }
    end
    local message = json.decode(response.body)
    if message.code ~= XCode.Successful then
        return message.code
    end
    return message.code, message.data
end
return LoginComponent