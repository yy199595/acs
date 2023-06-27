local Player = {}
local PlayerCounter = 10000
local log = require("Log")
local log_error = require("Log").Error
function Player:OnInit(info)

    self.updates = {}
    self.player_id = 0
    self.components = { }
    self.app = require("App")
    self.account = info.account
    self.passwd = info.passwd
    assert(self.account)
    assert(self.passwd)

    self:AddComponent("LoginComponent")
    self:AddComponent("ChatComponent")
end

function Player:AddComponent(name)
    local component = Class(name)
    if component == nil then
        log_error(string.format("add %s fail", name))
        return false
    end
    component.player = self
    if component["OnUpdate"] ~= nil then
        self.updates[name] = component
    end
    self.components[name] = component
end

function Player:DelComponent(name)
    if self.updates[name] ~= nil then
        self.updates[name] = nil
    end
    if self.components[name] ~= nil then
        self.components[name] = nil
    end
end

function Player:_Invoke(name, ...)
    local func = self[name]
    if func ~= nil then
        local code, result = xpcall(func, log_error, self, ...)
        if not code then
            log_error(debug.traceback(), result)
        end
    end
    for _, component in pairs(self.components) do
        local func1 = component[name]
        if func1 ~= nil then
            local code, result = xpcall(func1, log_error, component, ...)
            if not code then
                log_error(debug.traceback(), result)
            end
        end
    end
end

function Player:OnLogin(data)

    self.token = data.token
    self.address = data.address
    self.player_id = PlayerCounter
    PlayerCounter = PlayerCounter + 1
    self.app:Make(self.player_id, self.address, "client")

    local code = self:Call("Gate.Login", self.token)
    if code ~= XCode.Successful then
        log.Error(self.account, " 登录网关服务器 [", self.address, "] 失败")
        return
    end
    self:_Invoke("OnLoginSuccess")
    log.Debug(self.account, " 登录网关服务器[", self.address, "]成功")
end

function Player:Call(func, message)
    return self.app:Call(self.player_id, func, message)
end

function Player:Send(func, message)
    return self.app:Send(self.player_id, func, message)
end

return Player