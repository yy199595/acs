

Main = {}
Main.Modules = { }

function GetModules()
    local modules = { }
    for _, module in pairs(package.loaded) do
        if type(module) == "table" then
            table.insert(modules, module)
        end
    end
    return modules
end

function Main.Awake()
    print("***************")
end

function Main.AllServiceStart()
    for i = 1, 10 do
        local account = string.format("%d@qq.com",1000 + i)
        local userInfo = MongoComponent.QueryOnce("user_account", {
            _id = account
        })

    end
end

function Main.OnLoadModule(moduleName)

    local oldModule = package.loaded[moduleName] or {}
    package.loaded[moduleName] = nil
    local newModule = require(moduleName)
    if type(newModule) == "table" then
        for k, member in pairs(newModule) do
            if type(member) == "function" then
                oldModule[k] = member
            elseif oldModule[k] == nil then
                oldModule[k] = member
            end
        end
    end
    package[moduleName] = oldModule
    Log.Info("load " .. moduleName .. " Successful")
    return oldModule
end

function Main.Hotfix()
    local modules = GetModules()
    for _, module in pairs(modules) do
        local method = module["Hotfix"]
        if type(method) == "function" then
            local state, err = pcall(method)
            if not state then
                Log.Error(err)
            end
        end
    end
end

return Main
