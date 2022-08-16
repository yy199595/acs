

Main = {}
function Main.Awake()
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto/message");
    messageComponent:Import("test.proto")
    local service = package.loaded["AccountService"]
    print("service = ", service, package.path)
   return true
end

function Main.Complete()

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
