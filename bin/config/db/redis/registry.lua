local registry = { }

local time = { }
local tab_name = "registry"
--local server_info = "server_info"

function time.now()
    local result = redis.call("TIME")
    return result[1]
end

function registry.add(request)
    request.ping_time = time.now()
    local json = cjson.encode(request)
    redis.replicate_commands()
    --redis.call("SADD", request.name, request.id)
    redis.call("HSET", tab_name, request.id, json)
    return 0
end

function registry.update(request)
    local id = tostring(request.id)
    local json = redis.call("HGET", tab_name, id)
    if json == nil then
        return nil
    end
    local data = cjson.decode(json)
    data.ping_time = os.time()
    redis.call("HSET", tab_name, id, cjson.encode(data))
    return 0
end

function registry.query(request)
    local response = { }
    --local now = redis.call("TIME")
    local values = redis.call("HVALS", tab_name)
    for _, json in ipairs(values) do
        local data = cjson.decode(json)
        --local t = now - data.ping_time
        if data.name == request.name then
            table.insert(response, json)
        end
    end
    return response
end

function registry.del(request)
    local id = tostring(request.id)
    return redis.call("HDEL", tab_name, id)
end

local func = KEYS[1]
local data = ARGV[1]
return registry[func](cjson.decode(data))
