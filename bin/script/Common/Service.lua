
RpcService = { }


local log = require("Log")
local rpcService = Service
function RpcService.New(name)
    local tab = { }
    tab.name = name
    setmetatable(tab, {__index = RpcService })
    return tab
end

function RpcService:GetAddress(userId)
    if userId ~= nil then
        return rpcService.GetTarget(userId, self.name)
    end
    local target = rpcService.AllotServer(self.name)
    if target == nil then
        log.Error("allot address failure : ", self.name)
        error(self.name, " allot address failure")
        return nil
    end
    return target
end

function RpcService:Call(target, userId, func, message)
    if target == nil then
        target = self:GetAddress(userId)
    end
    local method = string.format("%s.%s", self.name, func)
    return rpcService.Call(target, method, userId, message)
end

function RpcService:Send(target, userId, func, message)
    if target == nil then
        target = self:GetAddress(userId)
    end
    local method = string.format("%s.%s", self.name, func)
    return rpcService.Call(target, method, userId, message)
end



return RpcService