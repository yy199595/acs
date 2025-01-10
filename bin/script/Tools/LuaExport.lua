
local LuaExport = { }
local string_rep = string.rep
local string_format = string.format

function is_array(t)

    local count = 0
    for _, v in pairs(t) do
        count = count + 1
    end
    return count == #t;
end

local function serialize(t, indent)
    indent = indent or 0
    local result = "{\n"
    local padding = string_rep("  ", indent)  -- 每一层缩进两个空格
    local is_array = is_array(t)

    for k, v in pairs(t) do
        local key, value
        if is_array then
            -- 如果是数组，只输出值
            value = (type(v) == "table") and serialize(v, indent + 1) or (type(v) == "string" and string_format("%q", v) or tostring(v))
            result = result .. padding .. "  " .. value .. ",\n"
        else
            -- 如果是键值对表，输出键和值
            key = (type(k) == "string") and string_format("[%q]", k) or string_format("[%s]", k)
            value = (type(v) == "table") and serialize(v, indent + 1) or (type(v) == "string" and string_format("%q", v) or tostring(v))
            result = result .. padding .. "  " .. key .. " = " .. value .. ",\n"
        end
    end

    result = result .. padding .. "}"
    return result
end


function LuaExport.Run(documents)
    local result = "return \n"
    return result .. serialize(documents, 1) .. "\n"
end

return LuaExport