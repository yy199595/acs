local timer = Timer
local TimerComponent = { }
local log = require("Log")
TimerComponent.Delay = function(ms, func)
    local id = timer.Add(ms, func)
    if id == nil then
        log.Error("add timer error")
    end
    return id
end

TimerComponent.Remove = function(id)
    return timer.Remove(id)
end

local loop = function(ms, func, count)
    if count == 0 then
        while true do
            coroutine.sleep(ms)
            func()
        end
        return
    end
    for i = 1, count do
        coroutine.sleep(ms)
        func()
    end
end

TimerComponent.Invoke = function(ms, func, count)

    coroutine.start()
end
