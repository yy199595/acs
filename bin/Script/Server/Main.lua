

Main = {}
function Main.Awake()
    for i, v in pairs(_G) do
        print(i, v)
    end
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

    local t1 = Time.GetNowSecTime();
    coroutine.sleep(1000)
    --Timer.AddTimer(5000, function()
    --    local t2 = Time.GetNowSecTime();
    --    print("&&&&&&&&&&&&&&&&&&&&&", t2 - t1)
    --end)

    --Sentry.Sleep(1500)
    return true
end
