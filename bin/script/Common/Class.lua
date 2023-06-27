local log = require("Log")
local log_error = log.Error
function OnError(...)
    log_error(...)
end

function Class(base, ...)

    local class = {
        OnInit = function(...)

        end
    }
    if base ~= nil then
        local meta = require(base)
        if type(meta) ~= "table" then
            return nil
        end
        for key, val in pairs(meta) do
            class[key] = val
        end
    end

    class.OnInit(class, ...)
    return class
end
