
local tab_insert = table.insert
local tab_unpack = table.unpack
local log_err = require("Log").Error
local Component = require("Component")

local EventComponent = Component()

SetMember(EventComponent, "events", { })

function EventComponent:Add(eve, obj, func)
    local method = obj[func]
    if type(method) ~= "function" then
        log_err(debug.traceback())
        return false
    end
    local events = self.events[eve]
    if events == nil then
        events = { }
        self.events[eve] = events
    end
    tab_insert(events, { obj, func })
    return true
end



function EventComponent:Trigger(eve, ...)
    local events = self.events[eve]
    if events == nil then
        return
    end
    for _, info in ipairs(events) do
        local obj, name = tab_unpack(info)
        local func = obj[name]
        func(obj, ...)
    end
end

function EventComponent:AddOnce(eve, obj, func)
    local method = obj[func]
    if type(method) ~= "function" then
        log_err(debug.traceback())
        return false
    end
    self.events[eve] = { obj, func }
end

function EventComponent:TriggerOnce(eve, ...)
    local info = self.events[eve]
    if info == nil then
        return XCode.Failure
    end
    local obj, name = tab_unpack(info)
    local func = obj[name]
    return pcall(func, obj, ...)
end

return EventComponent