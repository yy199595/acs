
local log = require("Log")

local redis = require("RedisComponent")
local mongo = require("MongoComponent")
local RpcService = require("RpcService")

local Chat = RpcService()

SetMember(Chat, "count", 0)
SetMember(Chat, "players", { })

function Chat:OnLogin(userId)
    log.Debug("玩家%d登录聊天服务", userId)
    redis:Run("HSET", "player.chat", userId, os.time())
    mongo:InsertOnce("player.chat", { _id = userId, time = os.time() })
end

function Chat:Sleep()
    coroutine.sleep(200)
    return XCode.Ok
end

function Chat:Ping()
    return XCode.Ok
end

function Chat:OnPing()
    return XCode.Ok
end

function Chat:Chat(request)
    local nowTime = os.time()
    table.print(request)
    local playerId = request.head:Get("id")
    local redisResponse = redis:Run("HGET", "player.chat", playerId)

    if redisResponse ~= nil then
        local lastChatTime = tonumber(redisResponse) or 0
        if nowTime - lastChatTime < 2 then
            return XCode.CallFrequently
        end
    end

    redis:Run("HSET", "player.chat", playerId, nowTime)
    --self.app:SendToClient(playerId, "ChatComponent.ChatSystem", request.data)
    return XCode.Ok
end

function Chat:Request(request)
    --table.print(request)
end

function Chat:Ping(request)
    self.count = self.count + 2
    request.count = self.count
    return XCode.Ok
end

return Chat