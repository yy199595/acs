function coroutine.wakeup(cor, ...)
    local res, ret = coroutine.resume(cor, ...)
    assert(res, ret)
    return ret
end

function coroutine.call(func)
    local luaTaskSource = WaitLuaTaskSource.New()
    coroutine.start(function(taskSource)
        local state, ret = pcall(func)
        if not state then
            Log.Error(ret)
        end
        taskSource:SetResult(ret)
    end, luaTaskSource)
    return luaTaskSource
end


function coroutine.rpc(func, id, request, taskSource)
    --table.print(request)
    local context = function(luaTaskSource)
        local state, error, response = pcall(func, id, request)
        if not state then
            Log.Error("rpc error : ", error)
            Log.Error(debug.traceback())
            luaTaskSource:SetRpc(XCode.CallLuaFunctionFail, error)
        else
            luaTaskSource:SetRpc(error, response)
        end
    end
    coroutine.start(context, taskSource)
end

function coroutine.http(func, request, taskSource)
    local context = function(luaTaskSource)
        local tab = {}
        local state, error, response = pcall(func, request)
        if not state then
            tab.error = error
            tab.code = XCode.CallLuaFunctionFail
            Log.Error("http error : ", error)
            Log.Error(debug.traceback())
        else
            tab.code = error
            tab.data = response
        end
        print("--------------")
        luaTaskSource:SetHttp(tab.code, tab)
    end
    coroutine.start(context, taskSource)
end
