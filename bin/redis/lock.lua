local lock = {}
lock.lock = function(request)
    if redis.call("SETNX", request.key, 1) == 0 then
        return false
    end
    redis.call("EXPIRE", request.key, request.time)
    return true
end

local func = KEYS[1]
local response = {}
local request = cjson.decode(ARGV[1])
response.res = lock[func](request)

return cjson.encode(response)