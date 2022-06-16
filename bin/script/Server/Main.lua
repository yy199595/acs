

Main = {}
function Main.Awake()
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto");
    messageComponent:Import("test.proto")
    local timerComponent = App.GetComponent("TimerComponent")
    timerComponent:AddTimer(2000, function()
        print("--------------------")
    end)

    local functions = Main.GetComponentFuncs("Awake")
    for key, func in pairs(functions) do
        if not func() then
            Log.Error("Invoke [" .. key .. "] failure")
            return false
        end
        Log.Debug("Invoke [" .. key .. "] successful")
    end

   return true
end

function Main.GetComponentFuncs(func)
    local functions = {}
    for key, component in pairs(_G) do
        if string.find(key, "Component")
                or string.find(key,"Service") then
            if type(component[func]) == "function" then
                functions[string.format("%s.%s", key, func)] = component[func]
            end
        end
    end
    return functions
end

function Main.Start()
    coroutine.sleep(1.5)
    local redisComponent = RedisComponent
    Log.Error(redisComponent.Lock("yjz", 10))
    Log.Error(redisComponent.Run("main", "SET", "yjz", { name="yjz", age = 10 }))
    local functions = Main.GetComponentFuncs("Start")
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
    local functions = Main.GetComponentFuncs("Complete")
    for key, func in pairs(functions) do
        func()
        Log.Debug("Invoke [" .. key .. "] successful")
    end
end

function Main.Hotfix()

end

function Main.AllServiceStart()
    local functions = Main.GetComponentFuncs("AllServiceStart")
    for key, func in pairs(functions) do
        func()
        Log.Debug("Invoke [" .. key .. "] successful")
    end
end
