

function coroutine.wakeup(cor, ...)
    local res, ret = coroutine.resume(cor, ...)
    assert(res, ret)
    return ret
end

function coroutine.call(func, ...)
    local luaTaskSource = WaitLuaTaskSource.New()
    coroutine.start(function(taskSource, ...)
        local state, ret = pcall(func, ...)
        if not state then
            Log.LuaError(ret)
        end
        taskSource:SetResult(ret)
    end, luaTaskSource, ...)
    return luaTaskSource
end


function coroutine.rpc(func, id, request, taskSource)
    local context = function()
        local state, error, response = pcall(func, id, request)
        if not state then
            Log.LuaError(error);
            taskSource:SetRpc(XCode.CallLuaFunctionFail, error)
        else
            taskSource:SetRpc(error, response)
        end
    end
    coroutine.start(context, taskSource)
end

function coroutine.http(func, request, taskSource)
    local context = function()
        local response = {}
        local state, error = pcall(func, request, response)
        if not state then
            Log.LuaError(error);
            response.error = error
            response.code = XCode.CallLuaFunctionFail
        else
            response.code = error
        end
        taskSource:SetHttp(response.code, response)
    end
    coroutine.start(context)
end
