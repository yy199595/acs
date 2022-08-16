
local RedisComponent = {}
local component = App.GetComponent("MainRedisComponent")

function RedisComponent.Run(name, cmd, ...)
    assert(type(cmd) == "string")
    assert(type(name) == "string")
    return component:Run(name, cmd, table.pack(...))
end

function RedisComponent.AddCounter(name, key)
    assert(type(key) == "string")
    assert(type(name) == "string")
    local response = component.Run(name, "INCRBY", key, 1)
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

function RedisComponent.Unlock(key)

end
return RedisComponent