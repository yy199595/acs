
local this = _G.Timer
local TimerComponent = Class("Component")

---@param ms number
---@param func function
---@return number
function TimerComponent:AddTimer(ms, func)
    return this.Add(ms, func)
end

---@param timerId number
function TimerComponent:DelTimer(timerId)
    return this.Remove(timerId)
end

return TimerComponent