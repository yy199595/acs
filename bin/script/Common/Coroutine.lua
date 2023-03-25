

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
        local state, error, response
        if id == 0 then
            state, error, response = pcall(func, request)
        else
            state, error, response = pcall(func, id, request)
        end
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
        local json = { }
        local state, error, response = pcall(func, request)
        if not state then
            json.error = error
            json.code = XCode.CallLuaFunctionFail
        else if error ~= XCode.Successful then
            json.code = error
            json.error = response
        else
            json.code =error
            json.error = "OK"
            json.data = response
        end
        end
        taskSource:SetHttp(json.code, json)
    end
    coroutine.start(context)
end
