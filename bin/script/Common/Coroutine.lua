local xpcall = _G.xpcall

local log_error = require("Log").OnError
local coroutine_start = coroutine.start
local new_task_source = WaitLuaTaskSource.New

function coroutine.call(class, name, ...)

    local func = class[name]
    if func == nil then
        return
    end
    local luaTaskSource = new_task_source()

    coroutine_start(function(...)
        local status, ret = xpcall(func, log_error, class, ...)
        if not status then
            luaTaskSource:SetResult(nil)
            return
        end
        luaTaskSource:SetResult(ret)
    end, ...)

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
        elseif error ~= XCode.Ok then
            json.code = error
            json.error = response
        else
            json.code = error
            json.error = "OK"
            json.data = response
        end
        taskSource:SetHttp(json.code, json)
    end
    coroutine_start(context)
end
