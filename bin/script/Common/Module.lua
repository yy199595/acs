
local Module = { }
local modules = { }
local pcall = xpcall

require("Class")
local log = require("Log")
local log_err = log.Error
local file = require("util.fs")
local cor_new = coroutine.create
local cor_resume = coroutine.resume

local context = function(class, func, task, ...)
    local response = pcall(func, log_err, class, ...)
    print(task)
    task:SetResult(response)
end

function Module:Await(name, taskSource, ...)

    local func = self[name]
    if func == nil then
        return false
    end
    local co = cor_new(context)
    cor_resume(co, self, func, taskSource, ...)
    return true
end

return function(...)
    local info = debug.getinfo(2, "S")
    local module = modules[info.short_src]
    if module == nil then
        module = { }
        setmetatable(module, Module)
        for _, name in ipairs({...}) do
            local meta = require(name)
            if meta ~= nil then
                setmetatable(module, meta)
            end
        end
        local name = file.GetFileName(info.short_src)

        module.__name = name
        module.SetMember = SetMember
        module.__source = info.short_src
        modules[info.short_src] = module
        return module
    end
    for k, v in pairs(module) do
        if type(v) == "function" then
            module[k] = nil
        end
    end

    return module
end