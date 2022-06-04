
Service = {}

function Service.Call(func, id, request, taskSource)
    local context = function(luaTaskSource)
        local state, error, response = pcall(func, id, request)
        if not state then
            Log.Error(error)
            luaTaskSource:SetResult(XCode.CallLuaFunctionFail, error)
        else
            luaTaskSource:SetResult(error, response)
        end
    end
    coroutine.start(context, taskSource)
    return taskSource;
end