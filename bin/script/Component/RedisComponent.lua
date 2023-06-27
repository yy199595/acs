

local type = _G.type
local xpcall = xpcall
local assert = _G.assert
local json = require("json")
local tab_pack = table.pack
local log_error = require("Log").OnError

local RedisComponent = Class("Component")

RedisComponent.redis = require("Redis")

---@param cmd string
function RedisComponent:Run(cmd, ...)
    local func = self.redis.Run
    local parameter = tab_pack(...)
    local status, response = xpcall(func, log_error, cmd, parameter)
    if not status then
        return
    end
    return response
end

---@param cmd string
function RedisComponent:Send(cmd, ...)
    local func = self.redis.Send
    local parameter = tab_pack(...)
    local status, response = xpcall(func, log_error, cmd, parameter)
    if not status then
        return
    end
    return response
end

---@param cmd string
function RedisComponent:SyncRun(cmd, ...)
    local func = self.redis.Sync
    local parameter = tab_pack(...)
    local status, response = xpcall(func, log_error, cmd, parameter)
    if not status then
        return
    end
    return response
end

---@param key string
---@return number
function RedisComponent:AddCounter(key, start)
    local response = self:Run("INCR", key)
    if response ~= nil then
        return response + start or 0
    end
    return -1
end

---@param key string
---@return number
function RedisComponent:SubCounter(key)
    local response = self:Run("DECR", key)
    return type(response) == "number" and response or 0
end

---@param lua string xxx.xxx
---@param tab table
---@param async boolean
function RedisComponent:Call(name, tab, async)
    local func = self.redis.Call
    local status, response = xpcall(func, log_error, name, tab, async)
    if not status then
        return
    end
    return response
end

function RedisComponent:Lock(key, time)
    assert(type(key) == "string")
    assert(type(time) == "number")
    local response = self.redis.Call("lock.lock", {
        key = key, time = time
    })
    return json.decode(response).res
end

---@param key string
---@param value any
---@param second number
function RedisComponent:Set(key, value, second)

    if type(value) == "table" then
        value = json.encode(value)
    end
    if type(second) == "number" and second > 0 then
       return self:Run("SETEX", key, second, value)
    else
       return self:Run("SET", key, value)
    end
end

---@param key string
---@param tab boolean
function RedisComponent:Get(key, tab)
    local response = self:Run("GET", key)
    if type(response) == "string" and tab then
        return json.decode(response)
    end
    return response
end

return RedisComponent