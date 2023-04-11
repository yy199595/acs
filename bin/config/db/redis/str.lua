local str = {}

function str.timeout(request)
    redis.call("SET", request.ke, request.value)
    redis.call("EXPIRE", request.key, request.time)
    return true
end

local result = {}
local method = str[KEYS[1]]
local request = cjson.decode(ARGV[1])
local state, err, response = pcall(method, request)
if not state then
      error(err)
      result.error = err
else
      result.data = response
end
return cjson.encode(result)