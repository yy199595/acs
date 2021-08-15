ServiceProxy = {}
require("TableUtil")
local json = require("JsonUtil")

function ServiceProxy.LocalInvoke(service, method, operId, callbakcId, messageData)
    local data = messageData
    if type(messageData) == "string" then
        data = json.ToObject(messageData)
    end

    local action = service[method]

    if type(action) ~= "function" then
        return false
    end
    SoEasy.Warning(service, method, operId, callbakcId, data)
    local cor = coroutine.create(function(func, id, cb, message)
        local res, code, ret = pcall(func, id, message)
        if not res then
            assert(res, code)
        elseif callbakcId ~= 0 then
            if type(ret) == "table" then
                data = json.ToString(ret)
            end
            SoEasy.LuaRetMessage(cb, id, code, ret)
        end
    end)
    coroutine.resume(cor, action, operId, callbakcId, data)
    return true
end

function ServiceProxy.ProxyInvoke(service, method, address, operId, callbakcId, messageData)
    local data = messageData
    if type(messageData) == "string" then
        data = json.ToObject(messageData)
    end

    local action = service[method]

    if type(action) ~= "function" then
        return false
    end
    local cor = coroutine.create(function(func, addr, id, cb, message)
        local res, code, ret = pcall(func, id, message)
        if not res then
            assert(res, code)
        elseif callbakcId ~= 0 then
            if type(ret) == "table" then
                data = json.ToString(ret)
            end
            TableUtil.Print(ret)
            SoEasy.LuaRetMessage(addr, cb, id, code, ret)
        end
    end)
    coroutine.resume(cor, action, address, operId, callbakcId, data)
    return true
end

return ServiceProxy
