
Service = {}


function Service.Call(func, id, json)
    
    if json ~= nil then
        local request = Json.ToObject(json)
        if request == nil then
            return XCode.CallArgsError, ""
        end
        local code, response = func(id, request)
        if code == XCode.Successful and response then
            return code, Json.ToString(response)
        end
        return code or XCode.CallLuaFunctionFail, response or ""
    end
    local code, response = func(id)

    return func(id);
end

function Service.CallAsync(func, taskSouce, id, json)

    coroutine.start(function()
        local code, response = Service.Call(func, id, json)
        taskSouce:SetResult(code or XCode.CallLuaFunctionFail, responseJson)
    end)
end