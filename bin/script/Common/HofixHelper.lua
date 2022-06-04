HofixHelper = {}
HofixHelper.hofixModules = {}

function HofixHelper.LoadModuleByName(moduleName)
    local oldModule = package.loaded[moduleName] or {}
    package.loaded[moduleName] = nil
    require(moduleName)
    local newModule = package.loaded[moduleName]
    for k, member in pairs(newModule) do
        if type(member) == "function" then
            oldModule[k] = member
        end
    end
    package[moduleName] = oldModule
    Log.Info("load " .. moduleName .. " Successful")
    return oldModule
end
return HofixHelper
