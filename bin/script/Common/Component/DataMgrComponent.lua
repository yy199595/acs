DataMgrComponent = { }

local datas = { }
local MaxCount = 10000

local DataPool = { }
DataPool.name = { }
DataPool.keys = { }
DataPool.datas = { }
DataPool.Count = 0

function DataPool:RemoveKey(key)
    for i, val in ipairs(self.keys) do
        if val == key then
            table.remove(self.keys, i)
            return true
        end
    end
    return false
end

function DataPool:Remove(key)
    self:RemoveKey(key)
    if self.datas[key] ~= nil then
        self.datas[key] = nil
        self.Count = self.Count - 1
    end
end

function DataPool:Set(key, value)
    self:RemoveKey(key)
    self.datas[key] = value
    table.insert(self.keys,  key)
    while self.MaxCount >= MaxCount do
        self:Remove(self.keys[1])
    end
end

function DataPool:Get(key)
    local data = self.datas[key]
    if data ~= nil then
        self:RemoveKey(key)
        table.insert(self.keys,  key)
    end
    return data
end

function DataMgrComponent.GetPool(tab)
    local pool = datas[tab]
    if pool == nil then
        pool = { }
        setmetatable(pool, DataPool)
        datas[tab] = pool
    end
    return pool
end

function DataMgrComponent.Set(tab, id, value, insert)

    assert(type(tab) == "string")
    local pool = DataMgrComponent.GetPool(tab)

    pool:Set(id, value)
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
        MongoComponent.InsertOnce(tab, value)
    end
end

function DataMgrComponent.Get(tab, id, insert)
    assert(type(tab) == "string")
    local pool = DataMgrComponent.GetPool(tab)

    local result = pool:Get(id)
    if result ~= nil then
        return result
    end
    local _, pos = string.find('_', tab)
    if pos ~= nil then
        local db = string.sub(tab, 1, pos)
        local key = string.sub(tab, pos)
        local json = RedisComponent.Run(db, "HGET", key)
        if type(json) == "string" then
            return Json.Decode(json)
        end
        local response = MongoComponent.QueryOnce(tab, {
            _id = id
        })
        if response ~= nil then
            pool:Set(id, response)
            RedisComponent.Run(db, id, response)
        end
        return response
    end
    local response = MongoComponent.QueryOnce(tab, {
        _id = id
    })

    pool:Set(id, response)
    return response
end