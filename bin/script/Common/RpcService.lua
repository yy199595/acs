
require("XCode")
local type = _G.type
local xpcall = _G.xpcall
local app = require("App")
local cor_new = coroutine.create
local cor_resume = coroutine.resume
local log_error = require("Log").OnError
local CODE_OK = XCode.Ok
local CODE_CALL_FAIL = XCode.CallLuaFunctionFail
local CODE_NOT_EXIST = XCode.CallFunctionNotExist

local RpcService = { }
local rpc_services = { }

local rpc = function(class, func, request, taskSource)
	local status, code, response = xpcall(func, log_error, class, request)
	if not status then
		code = CODE_CALL_FAIL
		taskSource:SetRpc(CODE_CALL_FAIL, code)
		return
	end
	if code == nil then
		code = CODE_OK
	elseif type(code) == "table" then
		response = code
		code = CODE_OK
	end
	taskSource:SetRpc(code, response)
end

local async = function(class, func, task, ...)
	local _, response = xpcall(func, log_error, class, ...)
	task:SetResult(response)
end

function RpcService:Await(name, taskSource, ...)

	local func = self[name]
	if func == nil then
		return false
	end
	local co = cor_new(async)
	cor_resume(co, self, func, taskSource, ...)
	return true
end

function RpcService:__Call(name, request, taskSource)
	local func = self[name]
	if func == nil then
		return CODE_NOT_EXIST
	end
	local co = cor_new(rpc)
	cor_resume(co, self, func, request, taskSource)
end

function RpcService:__Invoke(name, request)
	local func = self[name]
	if func == nil then
		return CODE_NOT_EXIST
	end
	local status, code, response = xpcall(func, log_error, self, request)
	if not status then
		return CODE_CALL_FAIL
	end
	return code, response
end

return function()
	local info = debug.getinfo(2, "S")
	local service = rpc_services[info.short_src]
	if service == nil then
		service = { }
		service.app = app
		setmetatable(service, RpcService)
		service.__source = info.short_src
		rpc_services[info.short_src] = service
		return service
	end
	for k, v in pairs(service) do
		if type(v) == "function" then
			service[k] = nil
		end
	end
	return service
end