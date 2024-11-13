
local app = require("App")
local Module = require("Module")

local type = _G.type
local xpcall = xpcall
local http_services = { }
local cor_new = coroutine.create
local cor_resume = coroutine.resume
local log_error = require("Log").OnError

local CODE_OK = XCode.Ok
local CODE_CALL_FAIL = XCode.CallLuaFunctionFail
local CODE_NOT_EXIST = XCode.CallFunctionNotExist

local HttpService = Module()

local context = function(class, func, request, taskSource)
    local status, code, result = xpcall(func, log_error, class, request)
    if not status then
        code = CODE_CALL_FAIL
    end
    if type == nil then
        code = CODE_OK
    elseif type(code) == "table" then
        result = code
        code = CODE_OK
    end
    taskSource:SetHttp(code, result)
end

function HttpService:__Invoke(name, request)
    local func = self[name]
    if func == nil then
        return CODE_NOT_EXIST
    else
        local status, code, response = xpcall(func, log_error, self, request)
        if not status then
            return CODE_NOT_EXIST
        end
        return code, response
    end
end

function HttpService:__Call(name, request, taskSource)
    local func = self[name]
    if func == nil then
        taskSource:SetHttp(CODE_NOT_EXIST)
    else
        local co = cor_new(context)
        cor_resume(co, self, func, request, taskSource)
    end
end

local async = function(class, func, task, ...)
    local _, response = xpcall(func, log_error, class, ...)

    task:SetResult(response)
end

function HttpService:Await(name, taskSource, ...)

    local func = self[name]
    if func == nil then
        return false
    end
    local co = cor_new(async)
    cor_resume(co, self, func, taskSource, ...)
    return true
end

return function()
    local info = debug.getinfo(2, "S")
    local module = http_services[info.short_src]
    if module == nil then
        module = { }
        module.app = app
        module.SetMember = SetMember
        module.__source = info.short_src
        setmetatable(module, HttpService)
        http_services[info.short_src] = module
        return module
    end
    for k, v in pairs(module) do
        if type(v) == "function" then
            module[k] = nil
        end
    end
    return module
end