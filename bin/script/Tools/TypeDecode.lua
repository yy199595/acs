local TypeDecode = { }
local string_split = function(str, reps)
    local resultStrList = {}
    string.gsub(str,'[^'..reps..']+',function ( w )
        table.insert(resultStrList,w)
    end)
    return resultStrList
end

local json = require("util.json")

local split_array = function(value)
    local result = { }
    local flag = { ';', '|' }
    for _, tag in ipairs(flag) do
        local split = string_split(value, tag)
        if #split > 1 then
            for _, val in ipairs(split) do
                table.insert(result, val)
            end
            return result
        end
    end
    return { value }
end

TypeDecode["bool"] = function(value)
    local str = string.lower(value)
    return str == "true" or str == "1"
end

TypeDecode["json"] = function(value)
    local tab = json.decode(value)
    if tab == nil then
        return nil
    end
    return value
end

TypeDecode["int"] = function(value)
    return tonumber(value) or 0
end

TypeDecode["float"] = function(value)
    return tonumber(value) or 0
end

TypeDecode["double"] = function(value)
    return tonumber(value) or 0
end

TypeDecode["int[]"] = function(value)
    local result = { }
    local source = split_array(value)
    for _, val in ipairs(source) do
        table.insert(result, tonumber(val))
    end
    return result
end

TypeDecode["string"] = function(value)
    return value or ""
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
    return split_array(value)
end

TypeDecode["map"] = function(value)
    local result = { }
    local split = split_array(value)
    for _, val in ipairs(split) do
        local items = string_split(val, ":")
        if items == nil or #items ~= 2 then
            error("解析map结构失败" .. val)
        end
        local key = items[1]
        local content = items[2]
        result[key] = content
    end
    return result
end

TypeDecode["map<string,int>"] = function(value)
    local result = { }
    local split = split_array(value)
    for _, val in ipairs(split) do
        local items = string_split(val, ":")
        if items == nil or #items ~= 2 then
            error("解析map结构失败" .. val)
        end
        local key = items[1]
        local content = tonumber(items[2])
        result[key] = content
    end
    return result
end

return TypeDecode