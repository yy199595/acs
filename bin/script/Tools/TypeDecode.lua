local TypeDecode = { }
require("StringUtil")
TypeDecode.int = function(value)
    return tonumber(value)
end

TypeDecode["int[]"] = function(value)
    local result = { }
    local split = string.split(value, ";")
    for _, val in ipairs(split) do
        table.insert(result, tonumber(val))
    end
    return result
end

TypeDecode["string"] = function(value)
    return value
end

TypeDecode["time"] = function(value)
    local pattern = "(%d+)%-(%d+)%-(%d+) (%d+):(%d+):(%d+)"
    local year, month, day, hour, min, sec = value:match(pattern)
    local time_table = {
        year = year,
        month = month,
        day = day,
        hour = hour,
        min = min,
        sec = sec
    }
    return os.time(time_table)
end

TypeDecode["string[]"] = function(value)
    local result = { }
    local split = string.split(value, ";")
    for _, val in ipairs(split) do
        table.insert(result, val)
    end
    return result
end

TypeDecode["map"] = function(value)
    local result = { }
    local split = string.split(value, "|")
    for _, val in ipairs(split) do
        local items = string.split(val, ":")
        if items == nil or #items ~= 2 then
            error("解析map结构失败" .. val)
        end
        local key = items[1]
        local content = items[2]
        local number = tonumber(content)
        if number then
            result[key] = number
        else
            result[key] = content
        end
    end
    return result
end

TypeDecode["map<int,int>"] = function(value)
    local result = { }
    local split = string.split(value, "|")
    for _, val in ipairs(split) do
        local items = string.split(val, ":")
        if items == nil or #items ~= 2 then
            error("解析map结构失败" .. val)
        end
        local key = tonumber(items[1])
        local content = tonumber(items[2])
        result[key] = content
    end
    return result
end

return TypeDecode