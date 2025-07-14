

local Module = require("Module")

require("Coroutine")
local log = assert(require("Log"))
local node = assert(require("Node"))
local RedisRegistry = assert(Module())
local json = assert(require("util.json"))
local redis = assert(require("RedisComponent"))
local redis_sub = assert(require("RedisSubComponent"))

function RedisRegistry:OnStart()
    redis_sub:Sub("NodeSystem.Add")
    redis_sub:Sub("NodeSystem.Del")
end

function RedisRegistry:OnComplete()
    self:OnHotfix()
    local message = node:GetInfo()
    message.last_time = os.time()
    redis:Run("HSET", "registry", message.id, message)
    redis:Publish("NodeSystem.Add", message)
end

function RedisRegistry:OnUpdate(tick)
    if tick % 15 == 0 then
        coroutine.start(function()
            local message = node:GetInfo()
            message.last_time = os.time()
            redis:Run("HSET", "registry", message.id, message)
        end)
    end
end

function RedisRegistry:OnHotfix()
    local nowTime = os.time()
    local list = redis:Run("HGETALL", "registry")
    for i, info in ipairs(list) do
        if i % 2 == 0 then
            local item = json.decode(info)
            if nowTime - item.last_time < 30 then
                node:Create(item.id, item.name)
                for key, address in pairs(item.listen) do
                    node:AddListen(item.id, key, address)
                end
                print(item)
            end
        end
    end
end

function RedisRegistry:OnDestroy()
    local message = node:GetInfo()
    redis:Run("HDEL", "registry", message.id)
    redis:Publish("NodeSystem.Del", message.id)
    redis_sub:UnSub("NodeSystem.Add")
    redis_sub:UnSub("NodeSystem.Del")
end

return RedisRegistry
