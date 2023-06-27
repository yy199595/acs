
require("Class")
local xpcall = _G.xpcall
local coroutine = _G.coroutine
local log_error = require("Log").OnError
local CODE_CALL_FAIL = XCode.CallLuaFunctionFail
local CODE_NOT_EXIST = XCode.CallFunctionNotExist

local RpcService = Class("Component")

function RpcService:__Call(name, request, taskSource)
	local func = self[name]
	if func == nil then
		return CODE_NOT_EXIST
	end
	local context = function()
		local status, code, response = xpcall(func, log_error, self, request)
		if not status then
			taskSource:SetRpc(CODE_CALL_FAIL, code)
		else
			taskSource:SetRpc(code, response)
		end
	end
	coroutine.start(context, taskSource)
end

function RpcService:__Hotfix(tab)
	for key, val in pairs(tab) do
		if type(val) == "function" then
			self[key] = val
		end
	end
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

return RpcService