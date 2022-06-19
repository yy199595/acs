
local this
local self
RedisComponent = {}
function RedisComponent.Awake()
    this = RedisComponent
    self = App.GetComponent("MainRedisComponent")
    return this ~= nil
end

function RedisComponent.Run(name, cmd, ...)
    assert(type(cmd) == "string")
    assert(type(name) == "string")
    return self:Run(name, cmd, table.pack(...))
end

function RedisComponent.AddCounter(name, key)
    assert(type(key) == "string")
    assert(type(name) == "string")
    local response = this.Run(name, "INCRBY", key, 1)
    return type(response) == "number" and response or -1
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

function RedisComponent.Unlock(key)

end