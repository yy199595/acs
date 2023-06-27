
local App = { }

local xpcall = _G.xpcall
local this = require("Actor")
local log_err = require("Log").Stack

---@param actorId number
---@param name string
---@param request table
---@return number
function App:Send(actorId, name, request)
    local func = this.Send
    local status, code = xpcall(func, log_err, actorId, name, request)
    if not status then
        return XCode.CallLuaFunctionFail
    end
    return code
end

---@param actorId number
---@param name string
---@param request table
---@return number table
function App:Call(actorId, name, request)
    local func = this.Call
    local status, code, response = xpcall(func, log_err, actorId, name, request)
    if not status then
        return XCode.CallLuaFunctionFail
    end
    return code, response
end

---@param actorId number
---@param name string
---@return string
function App:GetListen(actorId, name)
    local func = this.GetListen
    local status, addr = xpcall(func, log_err, actorId, name)
    if not status then
        return
    end
    return addr
end


---@param playerId number
---@param name string
---@param request table
---@return number
function App:SendToClient(playerId, name, request)
    local func = this.SendToClient
    local status, code, result = xpcall(func, log_err, playerId, name, request)
    if not status then
        return XCode.CallArgsError
    end
    return code, result
end

---@param service string
---@return string
function App:Allot(service)
    local func = this.Random
    local status, id = xpcall(func, log_err, service)
    if not status then
        return
    end
    return id
end

---@param id number
---@param addr string
---@param name string
function App:Make(id, addr, name)
    local func = this.NewServer
    xpcall(func, log_err, id, addr, name)
end

---@param service string
function App:AddWatch(service)
    local func = this.AddWatch
    xpcall(func, log_err, service)
end

return App