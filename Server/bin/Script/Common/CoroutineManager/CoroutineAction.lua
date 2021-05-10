
CoroutineAction = { }

function CoroutineAction.Invoke(action, session, operId, callbakcId, messageData)
    local address = session:GetAddress()
    if type(messageData) == 'string' then

    end
    local code, data = action(session, operId, callbakcId, messageData) 
    if callbakcId ~= 0 then
        if type(data) == "table" then
            local json = require("Util.JsonUtil")
            data = json.ToString(data)
        end
        SoEasy.SendByAddress(address, callbakcId, code, data)
    end
end

return CoroutineAction