

Main = {}
function Main.Awake()
    table.print(ServerConfig)
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto");
    messageComponent:Import("test.proto")
    local timerComponent = App.GetComponent("TimerComponent")
    timerComponent:AddTimer(2000, function()
        print("--------------------")
    end)
   return true
end

function Main.Start()
    coroutine.sleep(1.5)
    return true
end
