

Main = {}

function Main.Awake()
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto/message");
    messageComponent:Import("test.proto")
    local service = package.loaded["AccountService"]
    print("service = ", service)
    for key, value in pairs(package.loaded) do
        print(key, value, type(value))
    end

   return true
end


function Main.Start()
    local functions = Main.GetAllFunctions("Start")
    for key, func in pairs(functions) do
        if not func() then
            Log.Error("Invoke [" .. key .. "] failure")
            return false
        end
        Log.Debug("Invoke [" .. key .. "] successful")
    end
    return true
end

function Main.Complete()

    local functions = Main.GetAllFunctions("Complete")
    for key, func in pairs(functions) do
        func()
        Log.Debug("Invoke [" .. key .. "] successful")
    end
end

function Main.Hotfix()

end

function Main.AllServiceStart()
    local functions = Main.GetAllFunctions("AllServiceStart")
    for key, func in pairs(functions) do
        func()
        Log.Debug("Invoke [" .. key .. "] successful")
    end
    table.print(_G)
end

return Main
