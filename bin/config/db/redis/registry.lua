local registry = { }

local time = { }
local util = { }
local tab_key = "Registry"
--local server_info = "server_info"

function util.time_out(last, t)
    local result = redis.call("TIME")
    local now_time = result[1]
    return now_time - last > t
end

function util.get_key(name)
    return string.format("{0}.{1}", tab_key, name)
end

function time.now()
    local result = redis.call("TIME")
    return result[1]
end

function registry.add(request)
    request.ping_time = time.now()
    local json = cjson.encode(request)
    redis.replicate_commands()

    local rpc = request.listens.rpc
    local key = util.get_key(request.name)
    redis.call("HSET", key, rpc, json)
    return 0
end

function registry.clear()
    local keys = redis.call("HKEYS")
    return redis.call("HDEL", keys)
end

function registry.update(request)
    local rpc = request.listens.rpc
    local key = util.get_key(request.name)
    local json = redis.call("HGET", key, rpc)
    if json == nil then
        return nil
    end
    local data = cjson.decode(json)
    data.ping_time = os.time()
    redis.call("HSET", request.name, rpc, cjson.encode(data))
    return 0
end

function registry.query(request)
    local response = { }
    local key = util.get_key(request.name)
    local values = redis.call("HVALS", key)
    for _, json in ipairs(values) do
        --local data = cjson.decode(json)
        --local t = now - data.ping_time
        table.insert(response, json)
    end
    return response
end

function registry.del(request)
    local rpc = request.listens.rpc
    local key = util.get_key(request.name)
    return redis.call("HDEL", key, rpc)
end

local func = KEYS[1]
local data = ARGV[1] or "{}"
return registry[func](cjson.decode(data))
