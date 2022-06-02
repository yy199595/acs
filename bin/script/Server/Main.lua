

Main = {}
function Main.Awake()
    local messageComponent = App.GetComponent("MessageComponent")
    messageComponent:Load("./proto", {"test.proto"});
    messageComponent:New("lua.Test", {})
    local timerComponent = App.GetComponent("TimerComponent")
    timerComponent:AddTimer(2000, function()
        print("--------------------")
    end)
   return true
end

Person = {}
Person.age = 10
Person.name = "xiaoming"
Person.arr = {}



for i = 1, 10 do
    table.insert(Person.arr, i)
end

function Main.Start()
    coroutine.sleep(1.5)
    return true
end
