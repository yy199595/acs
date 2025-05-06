

require("Class")
require("XCode")
require("Coroutine")
local xpcall = _G.xpcall
local Module = require("Module")
local log_err = require("Log").Stack

local app = require("core.app")
local config = app.GetConfig()

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

---@param key string
---@return any
function App:GetConfig(key)
    if key ~= nil then
        return config[key]
    end
    return config
end

function App:GetPath(key)
    return app.GetPath(key)
end

return App