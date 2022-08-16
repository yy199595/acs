

Main = {}
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

function Main.OnLoadModule(name)
    print("load module " .. name)
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
