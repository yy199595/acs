
local components = { }
local app = require("App")

return function()
    local info = debug.getinfo(2, "S")
    local module = components[info.short_src]
    if module == nil then
        module = { }
        module.app = app
        module.__source = info.short_src
        components[info.short_src] = module
        return module
    end
    for k, v in pairs(module) do
        if type(v) == "function" then
            module[k] = nil
        end
    end
    return module
end
