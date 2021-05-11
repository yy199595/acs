
Action = { }

function Action.Invoke(action, address, operId, callbakcId, messageData)
    if type(messageData) == 'string' then
        local json = require("Util.JsonUtil")
        messageData = json.ToObject(messageData)
    end
    local cor = coroutine.create(function()
        local code, data = action(session, operId, callbakcId, messageData) 
        if callbakcId ~= 0 then
            if type(data) == "table" then
                local json = require("Util.JsonUtil")
                data = json.ToString(data)
            end
            SoEasy.SendByAddress(address, operId, callbakcId, code, data)
        end
    end)
    coroutine.resume(cor) 
end

return Action