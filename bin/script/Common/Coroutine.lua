local xpcall = _G.xpcall
local start_cor = coroutine.start
local log_error = require("Log").OnError
local new_task_source = WaitLuaTaskSource.New

function coroutine.wakeup(cor, ...)
    local res, ret = coroutine.resume(cor, ...)
    assert(res, ret)
    return ret
end

function coroutine.start(func, ...)
    local co = coroutine.create(func)
    coroutine.resume(co, ...)
    return co
end

local callback = function(func, taskSource, ...)

    local status, ret = xpcall(func, log_error, ...)
    if not status then
        taskSource:SetResult(nil)
        return
    end
    taskSource:SetResult(ret)
end

function coroutine.call(class, name, ...)
    local func = class[name]
    if func == nil then
        return
    end
    local luaTaskSource = new_task_source()
    start_cor(callback, func, luaTaskSource, class, ...)
    return luaTaskSource
end


function coroutine.rpc(func, request, taskSource)
    local context = function()
        local  state, error, response = pcall(func, request)
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
        elseif error ~= XCode.Successful then
            json.code = error
            json.error = response
        else
            json.code = error
            json.error = "OK"
            json.data = response
        end
        taskSource:SetHttp(json.code, json)
    end
    coroutine.start(context)
end
