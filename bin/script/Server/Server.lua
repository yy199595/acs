

Server = {}
Server.Modules = { }

function GetModules()
    local modules = { }
    for _, module in pairs(package.loaded) do
        if type(module) == "table" then
            table.insert(modules, module)
        end
    end
    return modules
end

function Server.Awake()
    print("***************")
end

function Server.AllServiceStart()

    local r1 = MongoComponent.InsertOnce("data_account", {
        _id = "646585122@qq.com",
        login_ip = "127.0.0.1",
        user_id = 1122,
        login_time = os.time(),
        register_time = os.time(),
        token = "0x00ssdjsaklj"
    })

    local r3 = MongoComponent.InsertOnce("data_account", {
        _id = "646585123@qq.com",
        login_ip = "127.0.0.1",
        user_id = 1122,
        login_time = os.time(),
        register_time = os.time(),
        token = "0x00ssdjsaklj"
    })

    local r2 = MongoComponent.Update("data_account", {
        id = "646585122@qq.com"
    }, {
        login_time_1 = 11223
    })

    local tab = { }
    table.insert(tab, {_id = "646585123@qq.com"})
    table.insert(tab, {_id = "646585122@qq.com"})
    local response = MongoComponent.Query("data_account", {
        _id = {
            ["$in"] = {
                "646585123@qq.com",
                "646585122@qq.com"
            }
        }
    })
    table.print(response)
end

function Server.OnLoadModule(moduleName)

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

function Server.Hotfix()
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

return Server
