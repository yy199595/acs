local str = {}

function str.timeout(request)
    redis.call("SET", request.key, request.value)
    redis.call("EXPIRE", request.key, request.time)
    return 1
end

local json = ARGV[1]
local method = str[KEYS[1]]
return method(cjson.decode(json))