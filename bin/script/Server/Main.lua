

Main = {}

function Main.Bson(str)
    print("-------------")
    local bson = Bson.Decode(str)
    table.print(bson)
    print("+++++++++++++++")
end
function Main.Awake()
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto");
    messageComponent:Import("test.proto")
    local timerComponent = App.GetComponent("TimerComponent")
    timerComponent:AddTimer(2000, function()
        print("--------------------")
    end)

    local functions = Main.GetAllFunctions("Awake")
    for key, func in pairs(functions) do
        if not func() then
            Log.Error("Invoke [" .. key .. "] failure")
            return false
        end
        Log.Debug("Invoke [" .. key .. "] successful")
    end

   return true
end

function Main.GetAllFunctions(func)
    local functions = {}
    for _, obj in pairs(_G) do
        if type(obj) == "table" and type(obj[func]) == "function" then
                functions[string.format("%s.%s", key, func)] = obj[func]
        end
    end
    return functions
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
    coroutine.sleep(0.5)
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
end
