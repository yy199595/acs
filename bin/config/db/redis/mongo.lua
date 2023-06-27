
local mongo = { }
function mongo.add(tab, key, data)
    local message = cjson.encode(data)
    local ret = redis.call("HSETNX", tab, key, message)
    if ret == 0 then
        return false
    end
    redis.call("ZADD ", "mongodb", "*", "add", message)
    return true
end

function mongo.find_one(tab, key)
    local ret = redis.call("HGET", tab, key)
    if ret == nil then
        return false
    end
    return true, ret
end

function mongo.update(tab, key, values)
    local ret = redis.call("HGET", tab, key)
    if ret == nil then
        return false
    end
    local result = cjson.decode(ret)
    for k, v in pairs(values) do
        result[k] = v
    end
    redis.call("HSET", tab, key, cjson.encode(result))
    local message = cjson.encode(values)
    redis.call("xadd ", "mongodb", "*", "update", message)
    return true
end

local func = KEYS[1]
local request = cjson.decode(ARGV[1])
local tab, key, data = request.tab, request.key, request.data

local code, response = mongo[func](tab, key, data)
return cjson.encode({
    code = code,
    data = response
})