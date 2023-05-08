---
--- Generated by EmmyLua(https://github.com/EmmyLua)
--- Created by leyi.
--- DateTime: 2023/5/8 20:17
---
local Console = { }

Console.LogDebug = 1
Console.LogInfo = 2
Console.LogWarn = 3
Console.LogError = 4
Console.LogFatal = 5

local json = rapidjson
local console = ConsoleLog
local table_pack = table.pack
local string_math = string.match
local string_fmt = string.format
local table_insert = table.insert
local table_concat = table.concat
local debug_getinfo = debug.getinfo
Console.FormatLog = function(runInfo, ...)
    local ret = { }
    local val = table_pack(...)
    local line = runInfo.currentline
    local name = string_math(runInfo.short_src, ".+/([^/]*%.%w+)$")
    table_insert(ret, string_fmt("%s:%d", name, line))

    for _, v in ipairs(val) do
        if type(v) == "table" then
            table_insert(ret, json.encode(v))
        else if type(v) == "string" then
            table_insert(ret, v)
        else
            table_insert(ret, tostring(v))
        end
        end
    end
    return table_concat(ret, " ")
end

function Console.Show(type, message)
    if os.debug then
        console.Show(type, message)
    end
end

function Console.Info(...)
    if os.debug then
        local runInfo = debug_getinfo(2)
        console.Show(Console.LogInfo, Console.FormatLog(runInfo, ...))
    end
end

function Console.Debug(...)
    if os.debug then
        local runInfo = debug_getinfo(2)
        console.Show(Console.LogDebug, Console.FormatLog(runInfo, ...))
    end
end

function Console.Warning(...)
    if os.debug then
        local runInfo = debug_getinfo(2)
        console.Show(Console.LogWarn, Console.FormatLog(runInfo, ...))
    end
end

function Console.Error(...)
    if os.debug then
        local runInfo = debug_getinfo(2)
        console.Show(Console.LogError, Console.FormatLog(runInfo, ...))
    end
end

function Console.Fatal(...)
    if os.debug then
        local runInfo = debug_getinfo(2)
        console.Output(Console.LogFatal, Console.FormatLog(runInfo, ...))
    end
end

return Console