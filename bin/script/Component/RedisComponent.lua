local type = _G.type
local tab_pack = table.pack
local tab_insert = table.insert
local str_format = string.format
local json = require("util.json")
local Component = require("Component")
local redis = require("db.redis")

local RedisComponent = Component()

---@param cmd string
function RedisComponent:Run(cmd, ...)
    local parameter = tab_pack(...)
    return redis.Run(cmd, parameter)
end

---@param cmd string
function RedisComponent:Send(cmd, ...)
    local parameter = tab_pack(...)
    return redis.Send(cmd, parameter)
end

---@param cmd string
function RedisComponent:SyncRun(cmd, ...)
    local parameter = tab_pack(...)
    return redis.Sync(cmd, parameter)
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

---@param name string xxx.xxx
---@param req table
---@param async boolean
function RedisComponent:Call(name, req, async)
    local response = redis.Call(name, req, async)
    if response and #response > 0 then
        return json.decode(response)
    end
    return response
end

function RedisComponent:Lock(key, time)
    local response = redis.Call("lock.lock", {
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

function RedisComponent:ClearAll()
    self:Run("FLUSHALL")
end

---@param key string
---@param timeout number
---@return boolean
function RedisComponent:Lock(key, timeout)
    local response = self:Call("lock.lock", {
        key = key, time = timeout
    })
    return response and response == 1
end

---@param key string
function RedisComponent:UnLock(key)
    local response = self:Call("lock.unlock", {
        key = key
    })
    return response and response == 1
end

---@param tab string
---@param id any
---@param value table
function RedisComponent:HashSet(tab, id, value)
    local items = { }
    tab_insert(items, str_format("%s:%s", tab, id))
    for k, v in pairs(value) do
        tab_insert(items, k)
        tab_insert(items, v)
    end
    return redis.Run("HMSET", items)
end

---@param tab string
---@param id any
---@param field string
---@param value number
function RedisComponent:HashInc(tab, id, field, value)
    local key = str_format("%s:%s", tab, id)
    return redis.Run("HINCRBY", tab_pack(key, field, value or 1))
end

---@param tab string
---@param id any
---@param field string
---@param value any
function RedisComponent:HashUpdate(tab, id, field, value)
    if value == nil then
        return
    end
    local key = str_format("%s:%s", tab, id)
    return redis.Run("HSET", tab_pack(key, field, value))
end

---@param tab string
---@param message any
function RedisComponent:ListPush(tab, id, message)
    local key = str_format("%s:%s", tab, id)
    return redis.Run("RPUSH", tab_pack(key, message))
end

---@param tab string
---@param id any
---@param start number
---@param stop number
function RedisComponent:ListRange(tab, id, start, stop)
    local key = str_format("%s:%s", tab, id)
    return redis.Run("LRANGE", tab_pack(key, start, stop))
end

function RedisComponent:ListLen(tab, id)
    local key = str_format("%s:%s", tab, id)
    return redis.Run("LLEN", tab_pack(key))
end

function RedisComponent:For(start, pattern)
    local res = self:Run("SCAN", start, "MATCH", pattern)
    if #res == 0 then
        return 0, { }
    end
    local count = tonumber(res[1])
    table.remove(res, 1)
    return count, res
end

function RedisComponent:Publish(channel, message)
	local data = message
    if type(message) == "table" then
        data = json.encode(message)
    end
    return self:Run("PUBLISH", channel, data)
end


---@param channel string
---@return boolean
function RedisComponent:Sub(channel)
    return redis.Sub(channel)
end

---@param channel string
function RedisComponent:UnSub(channel)
    return redis.UnSub(channel)
end

return RedisComponent