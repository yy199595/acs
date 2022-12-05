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
    local context = function()
        local state, error, response = pcall(func, id, request)
        if not state then
            Log.SystemError(error);
            taskSource:SetRpc(XCode.CallLuaFunctionFail, error)
        else
            taskSource:SetRpc(error, response)
        end
    end
    coroutine.start(context, taskSource)
end

function coroutine.http(func, request, taskSource)
    local context = function()
        local tab = {}
        local state, error, response = pcall(func, request)
        if not state then
            tab.error = error
            Log.SystemError(error);
            tab.code = XCode.CallLuaFunctionFail
        else
            tab.code = error
            tab.data = response
        end
        taskSource:SetHttp(tab.code, tab)
    end
    coroutine.start(context)
end
