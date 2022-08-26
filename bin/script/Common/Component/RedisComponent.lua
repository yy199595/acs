
RedisComponent = {}
local component = App.GetComponent("RedisDataComponent")
function RedisComponent.Run(name, cmd, ...)
    assert(type(cmd) == "string")
    assert(type(name) == "string")
    return component:Run(name, cmd, table.pack(...))
end

function RedisComponent.Send(name, cmd, ...)
    assert(type(cmd) == "string")
    assert(type(name) == "string")
    return component:Send(name, cmd, table.pack(...))
end

function RedisComponent.AddCounter(name, key)
    assert(type(key) == "string")
    assert(type(name) == "string")
    local response = RedisComponent.Run(name, "INCRBY", key, 1)
    return type(response) == "number" and response or -1
end

function RedisComponent.Call(name, lua, tab)
    assert(type(tab) == "table")
    assert(type(lua) == "string")
    assert(type(name) == "string")
    local response = component:Call(name, lua, tab)
    return Json.Decode(response)
end

function RedisComponent.Lock(key, time)
    assert(type(key) == "string")
    assert(type(time) == "number")
    local response = component:Call("main", "lock.lock", {
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
        RedisComponent.Run(name, "SETEX", key, second, value)
    else
        RedisComponent.Run(name, "SET", key, value)
    end
end

function RedisComponent.Get(name, key, tab)
    assert(type(key) == "string")
    local response = RedisComponent.Run(name, "GET", key)
    if type(response) == "string" then
        if tab then
            return Json.Decode(response)
        end
        return response
    end
    return nil
end

function RedisComponent.Unlock(key)

end
return RedisComponent