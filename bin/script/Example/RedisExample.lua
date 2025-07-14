local Module = require("Module")

local RedisExample = Module()

function RedisExample:OnStart()


    local redis = require("RedisComponent")

    print(redis:Run("SET", "user", "root"))

    print(redis:Run("GET", "user"))

    local userInfo = {
        "user_id", 1000,
        "open_id", string.range(16),
        "name", "xiaoming",
        "level", 1,
        "attr", {
            hp = 100,
            atk = 200
        },
        "create_time",
        os.time()
    }

    print(redis:Run("HMSET", "user_1000", table.unpack(userInfo)))

    print(redis:Run("HGETALL", "user_1000"))

    print(redis:Run("XADD", "rpc_message", "*", "user_1000", userInfo))

    print(redis:Run("XLEN", "rpc_message"))

    --print(redis:Run("XRANGE", "rpc_message", "-", "+"))


end

--local mq = require("RedisMQ")
--function RedisExample:OnUpdate(tick)
--
--    coroutine.start(function()
--        local response = mq:Read("rpc_message", 10, 1000)
--        if response ~= nil then
--            local items = { }
--            for _, info in ipairs(response) do
--                table.insert(items, info.id)
--            end
--            print(mq:Del("rpc_message", table.unpack(items)))
--        end
--    end)
--
--    coroutine.start(function()
--        coroutine.sleep(1000)
--        for i = 1, math.random(1, 5) do
--            mq:Add("rpc_message", "ChatSystem.Chat", {
--                user_id = 10 + i
--            })
--        end
--
--    end)
--end


return RedisExample