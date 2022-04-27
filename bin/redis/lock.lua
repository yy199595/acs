local lock = {}
lock.lock = function(key)
    local md5, val = redis.call("HGET", "lua", "json.lua")
    print(md5, luaJson)
    if redis.call("SETNX", key, 1) == 0 then
        print("redis lock " .. key .. "set faulure")
        return 0
    end
    return redis.call("SETEX", key, 5, 1)
end

lock.unlock = function(key)
    return redis.call("DEL", key)
end

local func = KEYS[1]
local key = KEYS[2]

return lock[func](key)