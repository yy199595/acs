
local TimerComponent = { }

local timer = require("core.timer")

---@param ms number
---@param func function
---@return number
function TimerComponent:Add(ms, func)
    return timer.Add(ms, func)
end

---@param timerId number
function TimerComponent:Del(timerId)
    return timer.Remove(timerId)
end

---@param ms number
---@param func function
---@return number
function TimerComponent:AddUpdate(ms, func)
    return timer.AddUpdate(timerId)
end

return TimerComponent