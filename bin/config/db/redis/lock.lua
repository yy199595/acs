local redis_call = redis.call
local str_format = string.format
local json_decode = cjson.decode

local lock = {}
lock.lock = function(request)
    local key = str_format("lock:%s", request.key)
    if redis_call("SETNX", key, 1) == 0 then
        return 0
    end
    return redis_call("EXPIRE", key, request.time)
end

lock.unlock = function(request)
    local key = str_format("lock:%s", request.key)
    return redis_call("DEL", key) > 0 and 1 or 0
end

local func = KEYS[1]
local json = ARGV[1]
local request = json_decode(json)
return lock[func](request)