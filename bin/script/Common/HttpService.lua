
require("Class")
local HttpService = Class("Component")
local log_error = require("Log").OnError

function HttpService:__Invoke(name, request)
    local func = self[name]
    local http_response = { }
    if func == nil then
        http_response.code = XCode.CallFunctionNotExist
    else
        local status, code, response = xpcall(func, log_error, self, request)
        if not status then
            http_response.code = XCode.CallLuaFunctionFail
        else
            http_response.code = code
            http_response.data = response
        end
    end
    return http_response.code, http_response
end

function HttpService:__Call(name, request, taskSource)

    local func = self[name]
    if func == nil then
        taskSource:SetHttp({
            code = XCode.CallLuaFunctionFail
        })
    else
        local context = function()
            local response = { }
            local status, code, result = xpcall(func, log_error, self, request)
            if not status then
                response.error = code
                response.code = XCode.CallLuaFunctionFail
            else
                response.code = code
                response.data = result
            end
            taskSource:SetHttp(response.code, response)
        end
        coroutine.start(context, taskSource)
    end

end

return HttpService