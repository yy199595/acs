
local log = require("Log")
local app = require("App")

local RpcService = require("RpcService")

local Chat = RpcService()

SetMember(Chat, "count", 0)
SetMember(Chat, "players", { })

function Chat:OnLogin(userId)

end

function Chat:Sleep()
    coroutine.sleep(200)
    return XCode.Ok
end

function Chat:Ping()
    return XCode.Ok
end

function Chat:OnPing(request)
    return XCode.Ok
end

function Chat:OnChat(request)

    return XCode.Ok
end

function Chat:Request(request)
    --table.print(request)
    return XCode.Ok
end

function Chat:Ping(request)
    self.count = self.count + 2
    request.count = self.count
    return XCode.Ok
end

return Chat