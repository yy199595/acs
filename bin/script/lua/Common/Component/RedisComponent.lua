
RedisComponent = {}
local this = RedisComponent
local self = App.GetComponent("RedisDataComponent")
function RedisComponent.Run(name, cmd, ...)
    assert(type(cmd) == "string")
    assert(type(name) == "string")
    return self:Run(name, cmd, table.pack(...))
end

function RedisComponent.Send(name, cmd, ...)
    assert(type(cmd) == "string")
    assert(type(name) == "string")
    return self:Send(name, cmd, table.pack(...))
end

function RedisComponent.AddCounter(key)
    assert(type(key) == "string")
    local response = this.Run("main", "INCR", key)
    return type(response) == "number" and response or 0
end

function RedisComponent.SubCounter(key)
    assert(type(key) == "string")
    local response = this.Run("main", "DECR", key)
    return type(response) == "number" and response or 0
end

function RedisComponent.Call(name, lua, tab)
    assert(type(tab) == "table")
    assert(type(lua) == "string")
    assert(type(name) == "string")
    local response = self:Call(name, lua, tab)
    return Json.Decode(response)
end

function RedisComponent.Lock(key, time)
    assert(type(key) == "string")
    assert(type(time) == "number")
    local response = self:Call("main", "lock.lock", {
                key = key, time = time
            })
    table.print(response)
    return Json.Decode(response).res
end

function RedisComponent.Set(name, key, value, second)

    assert(type(key) == "string")
    if type(value) == "table" then
        value = Json.Encode(value)
    end
    if type(second) == "number" and second > 0 then
        this.Run(name, "SETEX", key, second, value)
    else
        this.Run(name, "SET", key, value)
    end
end

function RedisComponent.Get(name, key, tab)
    assert(type(key) == "string")
    local response = this.Run(name, "GET", key)
    if type(response) == "string" then
        if tab then
            return Json.Decode(response)
        end
        return response
    end
    return nil
end

return RedisComponent