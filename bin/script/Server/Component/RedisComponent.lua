
RedisComponent = {}
local redisComponent
function RedisComponent.Awake()
    redisComponent = App.GetComponent("MainRedisComponent")
    return redisComponent ~= nil
end

function RedisComponent.Run(name, cmd, ...)
    local tab = table.pack(...)
    return redisComponent:Run(name, cmd, tab)
end

function RedisComponent.AddCounter(name, key)
    local response = RedisComponent.Run(name, "INCRBY", key, 1)
    return type(response) == "number" and response or -1
end

function RedisComponent.Call(name, lua, tab)
    local response = redisComponent:Call(name, lua, tab)
    return Json.Decode(response)
end

function RedisComponent.Lock(key, time)
    local response = redisComponent:Call("main", "lock.lock", {
                key = key, time = time
            })
    return Json.Decode(response).res
end

function RedisComponent.Unlock(key)

end