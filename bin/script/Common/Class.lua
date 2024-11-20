
local on_new_index = function(class, k, v)
    print("-------", class, k, v)
    class[k] = v
end

local to_string = function(class)
    return ""
end

Clone = function(base)
    local class = { }
    if base ~= nil then
        local super = base
        if type(base) == "string" then
            super = require(base)
        end
        for k, v in pairs(super) do
            class[k] = v
        end
    end
    return class
end

---@param class table
---@param key string
---@param value any
SetMember = function(class, key, value)
    if class[key] == nil then
        class[key] = value
    end
end

Class = function(base)
    local class = { }
    if base ~= nil then
        local super = base
        if type(base) == "string" then
            super = require(base)
        end
        for k, v in pairs(super) do
            class[k] = v
        end
    end
    class.SetMember = SetMember
    return class
end