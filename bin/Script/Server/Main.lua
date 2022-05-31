

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

function Main.Start()
    coroutine.sleep(1.5)
    return true
end
