
function RpcCall(func, id, request, taskSource)
    table.print(request)
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
    table.print(request)
    local context = function(luaTaskSource)
        local tab = {}
        local state, error, response = pcall(func, request)
        if not state then
            tab.error = error
            tab.code = XCode.CallLuaFunctionFail
            Log.Error("http async call error =>[", error, "]")
        else
            tab.code = error
            tab.data = response
        end
        luaTaskSource:SetResult(tab.code, tab)
    end
    coroutine.start(context, taskSource)
end