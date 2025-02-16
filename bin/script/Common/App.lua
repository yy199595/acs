

require("Class")
require("XCode")
require("Coroutine")
local xpcall = _G.xpcall
local Module = require("Module")
local log_err = require("Log").Stack

local app = require("core.app")
local config = app.GetConfig()
local router_send = app.Send
local router_call = app.Call
local router_publish = app.Publish

local App = Module()
SetMember(App, "agent", { })

function App:Invoke(name, ...)
    for _, component in pairs(self.components) do
        local func = component[name]
        if type(func) == "function" then
            xpcall(func, log_err, component, ...)
        end
    end
end

---@return number
function App:NewGuid()
    return app.NewGuid()
end

function App:Stop()
    return app.Stop()
end

---@return string
function App:NewUuid()
    return app.NewUuid()
end

---@param name string
---@return boolean
function App:HasComponent(name)
    return app.HasComponent(name)
end

---@param id number
---@param channel string
---@param message table
function App:Publish(actorId, channel, message)
    local status, code = xpcall(router_publish, log_err, actorId, channel, message)
    if not status then
        return XCode.CallLuaFunctionFail
    end
    return code
end

---@param key string
---@return any
function App:GetConfig(key)
    if key ~= nil then
        return config[key]
    end
    return config
end

---@param actorId number
---@param name string
---@param request table
---@return number
function App:Send(actorId, name, request)
    local status, code = xpcall(router_send, log_err, actorId, name, request)
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
    local status, code, response = xpcall(router_call, log_err, actorId, name, request)
    if not status then
        return XCode.CallLuaFunctionFail
    end
    return code, response
end

---@param service string
---@return string
function App:Allot(service)
    local func = app.Random
    local status, id = xpcall(func, log_err, service)
    if not status then
        return
    end
    return id
end

---@param name string
---@return table
function App:GetServers(name)
    return app.GetServers(name)
end

function App:GetPath(key)
    return app.GetPath(key)
end

---@param id number
---@param name string
---@param address string
function App:AddListen(id, name, address)
    return app.AddListen(id, name, address)
end

---@param id number
---@param name string
function App:GetListen(id, name)
    return app.GetListen(id, name)
end

---@param id number
---@param name string
---@param listens table
function App:MakeServer(id, name, listen)
    return app.MakeServer({
        id = id,
        name = name,
        listen = listen
    })
end

return App