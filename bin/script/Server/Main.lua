

Main = {}
Main.Modules = { }
function Main.Awake()
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto/message");
    messageComponent:Import("test.proto")
    local service = package.loaded["AccountService"]
    --table.print(_G)
   return true
end

function Main.Complete()

end

function Main.OnLoadModule(path)
    local module = require(path)
    print("module = " .. path, module)
end

function Main.Hotfix()
    print("--------", MongoComponent)
    _G.temp = { }
    for _, key in ipairs(Main.Modules) do
        if _G[key] ~= nil then
            _G.temp[key] = _G[key]
            _G[key] = nil
        end
    end
end

function Main.HotfixAfter()
    print("======", MongoComponent)
end

function Main.Init()
    require("Component.RedisComponent")
    require("Component.MongoComponent")
    require("Component.MongoComponent")
    table.print(package.loaded)
end

function Main.AllServiceStart()

end

return Main
