local DataMgrComponent = { }

function DataMgrComponent.Set(tab, id, value, insert)

    assert(type(tab) == "string")
    local _, pos = string.find('_', tab)
    if pos ~= nil then
        local db = string.sub(tab, 1, pos)
        local key = string.sub(tab, pos)
        RedisComponent.Run(db, "HDEL", key, id)
    end
    local code = MongoComponent.Update(tab, {
        _id = id
    }, value, nil)
    if code ~= XCode.Successful and insert then
        value._id = id
        return MongoComponent.InsertOnce(tab, value)
    end
    return XCode.Successful
end

function DataMgrComponent.Get(tab, id)
    local _, pos = string.find('_', tab)
    if pos ~= nil then
        local db = string.sub(tab, 1, pos)
        local key = string.sub(tab, pos)
        local json = RedisComponent.Run(db, "HGET", key)
        if type(json) == "string" then
            return Json.Decode(json)
        end
    end
    local response = MongoComponent.QueryOnce(tab, {
        _id = id
    })
    return response
end

return DataMgrComponent