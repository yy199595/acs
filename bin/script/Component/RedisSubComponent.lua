

local table_pack = table.pack
local RedisSubComponent = { }
local redis = require("db.redis.sub")

function RedisSubComponent:Sub(channel)
    return redis.run("SUBSCRIBE", table_pack(channel))
end

function RedisSubComponent:UnSub(channel)
    return redis.run("UNSUBSCRIBE", table_pack(channel))
end


return RedisSubComponent