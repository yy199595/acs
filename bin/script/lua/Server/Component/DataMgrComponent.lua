local DataMgrComponent = { }
local mongo = require("Component.MongoComponent")
local redis = require("Component.RedisComponent")
function DataMgrComponent.Set(tab, id, value, insert)

    assert(type(tab) == "string")
    local _, pos = string.find('_', tab)
    if pos ~= nil then
        local db = string.sub(tab, 1, pos)
        local key = string.sub(tab, pos)
        redis.Run(db, "HDEL", key, id)
    end
    local code = mongo.Update(tab, {
        _id = id
    }, value, nil)
    if code ~= XCode.Successful and insert then
        value._id = id
        return mongo.InsertOnce(tab, value)
    end
    return XCode.Successful
end

function DataMgrComponent.Get(tab, id)
    local _, pos = string.find('_', tab)
    if pos ~= nil then
        local db = string.sub(tab, 1, pos)
        local key = string.sub(tab, pos)
        local json = redis.Run(db, "HGET", key)
        if type(json) == "string" then
            return Json.Decode(json)
        end
    end
    local response = mongo.QueryOnce(tab, {
        _id = id
    })
    return response
end

return DataMgrComponent