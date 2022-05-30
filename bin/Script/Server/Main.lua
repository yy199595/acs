

Main = {}
function Main.Awake()
    for i, v in pairs(ChatService) do
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

    local t1 = Time.GetNowMilTime();
    local httpComponent = App.GetComponent("HttpComponent")
    local info = {
        city = "北京",
        key = "17b541d4fc517e3a29b4636ac2f9a74a"
    }
    local content = httpComponent:Post("http://apis.juhe.cn/simpleWeather/query", Json.Encode(info))
    for k, v in pairs(content.head) do
        Log.Error(k, v)
    end
    Log.Error("time = ", Time.GetNowMilTime() - t1)

    coroutine.sleep(1.5)
    --Timer.AddTimer(5000, function()
    --    local t2 = Time.GetNowSecTime();
    --    print("&&&&&&&&&&&&&&&&&&&&&", t2 - t1)
    --end)

    --Sentry.Sleep(1500)
    return true
end
