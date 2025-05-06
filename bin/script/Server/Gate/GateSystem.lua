

local log = assert(require("Log"))
local node = assert(require("Node"))
local redis = assert(require("RedisComponent"))
local RpcService = assert(require("RpcService"))

local GateSystem = RpcService()


function GateSystem:OnAwake()
   --redis:Sub("GateSystem.OnPlayerLogin")
end

function GateSystem:OnComplete()

end


function GateSystem:OnPlayerLogin(message)
    log.Debug("{}", message)
end

return GateSystem