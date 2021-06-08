RedisClient = {}
local this = RedisClient
function RedisClient.InvokeCommand(cmd, ...)
    if coroutine.running() == nil then
        SoEasy.Error("please use it in the coroutine")
        return nil
    end
    return SoEasy.InvokeRedisCommand(cmd, ...)
end

function RedisClient.SetValue(key, value)
    if type(value) == "table" then
        local json = require("Util.JsonUtil")
        value = json.ToString(value)
    end
    local queryData = this.InvokeCommand("SET", key, value)
    return queryData ~= nil and queryData.code == 0
end

function RedisClient.SetHashValue(tab, key, value)
    if type(value) == "table" then
        local json = require("Util.JsonUtil")
        value = json.ToString(value)
    end
    local queryData = this.InvokeCommand("HSET", tab, key, value)
    return queryData ~= nil and queryData.code == 0
end

function RedisClient.GetValue(key)
    local queryData = this.InvokeCommand("GET", key)
    if queryData == nil or queryData.code ~= 0 then
        return nil
    end
    return queryData.data
end

function RedisClient.GetHashValue(tab, key)
    local queryData = this.InvokeCommand("HGET", tab, key)
    if queryData == nil or queryData.code ~= 0 then
        return nil
    end
    return queryData.data
end

function RedisClient.SetTimeoutValue(key, value, ms)
    if type(value) == "table" then
        local json = require("Util.JsonUtil")
        value = json.ToString(value)
    end
    local queryData = this.InvokeCommand("PSETEX", key, ms, value)
    return queryData ~= nil and queryData.code == 0
end

function RedisClient.DelValue(key)
    local queryData = this.InvokeCommand("DEL", key)
    return queryData ~= nil and queryData.code == 0
end

function RedisClient.DelHashValue(tab, key)
    local queryData = this.InvokeCommand("HDEL", tab, key)
    return queryData ~= nil and queryData.code == 0
end

return RedisClient
