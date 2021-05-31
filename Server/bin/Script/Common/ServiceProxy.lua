
ServiceProxy = {}
local json = require("JsonUtil")

function ServiceProxy.Invoke(action, operId, callbakcId, messageData)
    local data = messageData
    if type(messageData) == "string" then
        data = json.ToObject(messageData)
    end
    local cor = coroutine.create(action)
    local res, code, ret = coroutine.resume(cor, operId, data)
    if not res then
        assert(res, code)
    elseif callbakcId ~= 0 then
        if type(ret) == "table" then
            data = json.ToString(ret)
        end
        SoEasy.LuaReplyMsg(callbakcId, operId, code, ret)
    end
end

function ServiceProxy.InvokeAddress(action, address, operId, callbakcId, messageData)

    local data = messageData
    if type(messageData) == 'string' then
        data = json.ToObject(messageData)
    end
    local cor = coroutine.create(action)
    local res, code, ret = coroutine.resume(cor, operId, data)
    if not res then
        assert(res, code)
    elseif type(ret) == "table" then
        data = json.ToString(ret)
    end
    SoEasy.LuaReplyMsg(address, callbakcId, operId , code, data)
end

return ServiceProxy