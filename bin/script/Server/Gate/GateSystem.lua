

local app = require("App")
local log = require("Log")
local redis = require("RedisComponent")
local RpcService = require("RpcService")

local GateSystem = RpcService()


function GateSystem:OnAwake()
   --redis:Sub("GateSystem.OnPlayerLogin")
end

function GateSystem:OnComplete()
    app:Publish(nil, "GateSystem.OnPlayerLogin", {
        user_id = 10002,
        time = os.time()
    })
end


function GateSystem:OnPlayerLogin(message)
    log.Debug("{}", message)
end

return GateSystem