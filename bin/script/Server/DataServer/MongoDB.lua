local MongoDB = Class("RpcService")
local redis = require("RedisComponent")
local mongo = require("MongoComponent")
function MongoDB:Start()
    local session = self.app:Allot("MongoDB")
    mongo:SetIndex("user.account", { "user_id"})
end

function MongoDB:PullDataFromRedis(count)
    local response = redis:Run("XREAD", "streams", "mongodb", count)
    table.print(response)
end

--function MongoDB:OnUpdate(tick)
--    local count = redis:SyncRun("XLEN", "mongodb")
--    print("============", count)
--    if count > 0 then
--        print("pull data from redis count = ", count)
--        coroutine.start(self.PullDataFromRedis, self, count)
--    end
--end
return MongoDB