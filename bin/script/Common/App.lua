
local modules = { }
function App.Make(name)
    if modules[name] == nil then
        local class = Class(name)
        modules[name] = class
    end
    return modules[name]
end