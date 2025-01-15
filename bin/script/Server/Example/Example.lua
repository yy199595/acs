
local log = require("Log")
local RpcService = require("RpcService")

local Example = RpcService()

function Example:Ping(request)
    print("==========")
    return XCode.Ok, "pong"
end

return Example