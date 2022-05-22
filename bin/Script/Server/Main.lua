

Main = {}
function Main.Awake()
   return true
end

Person = {}
Person.age = 10
Person.name = "xiaoming"
Person.arr = {}



for i = 1, 10 do
    table.insert(Person.arr, i)
end

function Main.Awake()
    local t1 = Time.GetNowMilTime()
    for i = 1, 100000 do
        local s1 = Bson.Encode(Person)
        local b1 = Bson.Decode(s1)
    end
    local t2 = Time.GetNowMilTime()
    print("bson = ", t2 - t1)
    for i = 1, 100000 do
        local s2 = Json.Encode(Person)
        local b2 = Json.Decode(s2)
    end
    local t3 = Time.GetNowMilTime()
    print("json = ", t3 - t2)

    for k, v in pairs(Bson) do
        print(k, " = ", v)
    end
    return true
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
