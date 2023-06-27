
local Main = {}
local mongo = require("MongoComponent")
local redis = require("RedisComponent")
function Main:Awake()
    return true
end

local count = 0

function Main:Insert()

    redis:Run("SET", "a", "b")

    mongo:InsertOnce("user.account_info", {
        _id = "123456@qq.com",
        user_id = 10,
        account = "123456@qq.com",
        passwd = "11223344",
        phone_num = 11223344,
        create_time = os.time(),
        register_time = os.ms()
    })
    count = count - 1
end

function Main:FindOne()
    --redis:Run("HSET", "yjz", 1, "{}")
    mongo:FindOne("user.account_info", {
        _id = "123456@qq.com"
    })
    count = count - 1

end

function Main:OnComplete()
    self:FindOne()
    mongo:Drop("user.account_info")
    --local t1 = os.ms()
    --for i = 1, 10 do
    --    count = count + 2
    --    coroutine.start(Main.Insert, self)
    --    coroutine.start(Main.FindOne, self)
    --end
    --while count > 0 do
    --    coroutine.sleep(100)
    --end
    --print("=========== ", os.ms() - t1)
end
return Main