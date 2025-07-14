
local log = require("Log")
local node = require("Node")
local Module = require("Module")

local SERVICE_NAME = "RegistryService"
local RegistryComponent = assert(Module())

function RegistryComponent:Awake()
    self.id = nil
end

function RegistryComponent:OnComplete()
    self.id = node:Allot(SERVICE_NAME)
    if self.id == nil then
        log.Error("all service:{} fail", SERVICE_NAME)
        return
    end
    local info = node:GetInfo()
    if node:Call(self.id, "RegistryService.Add", info) ~= XCode.Ok then
        log.Error("register service fail")
    end
end

function RegistryComponent:OnUpdate(tick)
    if tick > 0 and tick % 5 == 0 then
        local info = node:GetInfo()
        node:Send(self.id, "RegistryService.Ping", info.id)
    end
end

function RegistryComponent:OnDestroy()
    local info = node:GetInfo()
    node:Call(self.id, "RegistryService.Del", info.id)
end

return RegistryComponent