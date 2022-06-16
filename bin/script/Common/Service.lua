
function RpcCall(func, id, request, taskSource)
    local context = function(luaTaskSource)
        local state, error, response = pcall(func, id, request)
        if not state then
            Log.Error("rpc async call error =>[", error, "]")
            luaTaskSource:SetResult(XCode.CallLuaFunctionFail, error)
        else
            luaTaskSource:SetResult(error, response)
        end
    end
    coroutine.start(context, taskSource)
end

function HttpCall(func, request, taskSource)
    local context = function(luaTaskSource)
        local state, error, response = pcall(func, request)
        if not state then
            Log.Error("http async call error =>[", error, "]")
            luaTaskSource:SetResult(XCode.CallLuaFunctionFail, error)
        else
            luaTaskSource:SetResult(error, response)
        end
    end
    coroutine.start(context, taskSource)
end