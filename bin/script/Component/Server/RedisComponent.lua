
local RedisComponent = {}

local redis = Redis
local this = RedisComponent
function RedisComponent.Run(cmd, ...)
    assert(type(cmd) == "string")
    return redis.Run(cmd, table.pack(...))
end

function RedisComponent.Send(cmd, ...)
    assert(type(cmd) == "string")
    return redis.Send(cmd, table.pack(...))
end

function RedisComponent.AddCounter(key)
    assert(type(key) == "string")
    local response = this.Run("INCR", key)
    return type(response) == "number" and response or 0
end

function RedisComponent.SubCounter(key)
    assert(type(key) == "string")
    local response = this.Run( "DECR", key)
    return type(response) == "number" and response or 0
end

function RedisComponent.Call(lua, tab)
    assert(type(tab) == "table")
    assert(type(lua) == "string")
    local response = redis.Call(lua, tab)
    if response == nil then
        return nil
    end
    return rapidjson.decode(response)
end

function RedisComponent.Lock(key, time)
    assert(type(key) == "string")
    assert(type(time) == "number")
    local response = redis.Call("lock.lock", {
                key = key, time = time
            })
    table.print(response)
    return rapidjson.decode(response).res
end

function RedisComponent.Set(key, value, second)

    assert(type(key) == "string")
    if type(value) == "table" then
        value = rapidjson.encode(value)
    end
    if type(second) == "number" and second > 0 then
        this.Run("SETEX", key, second, value)
    else
        this.Run("SET", key, value)
    end
end

function RedisComponent.Get(key, tab)
    assert(type(key) == "string")
    local response = this.Run("GET", key)
    if type(response) == "string" then
        if tab then
            return rapidjson.decode(response)
        end
        return response
    end
    return nil
end

return RedisComponent