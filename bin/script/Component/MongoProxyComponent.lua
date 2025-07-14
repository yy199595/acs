

local MongoProxyComponent = { }

local node = require("Node")
local MONGO_READ_PROXY = "MongoReadProxy"
local MONGO_WRITE_PROXY = "MongoWriteProxy"

---@param tab string
---@param documents table
---@return number
function MongoProxyComponent:InsertBatch(tab, documents)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Insert", {
        tab = tab,
        list = documents
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param document table
---@return number
function MongoProxyComponent:InsertOne(tab, document)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Insert", {
        tab = tab,
        list = { document }
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param filter table
---@param document table
---@param cmd string
---@return number
function MongoProxyComponent:UpdateOne(tab, filter, document, cmd)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Update", {
        tab = tab,
        cmd = cmd,
        multi = false,
        filter = filter,
        document = document
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param filter table
---@return number
function MongoProxyComponent:DeleteOne(tab, filter)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Delete", {
          tab = tab,
          limit = 1,
          filter = filter
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param documents table
---@return number
function MongoProxyComponent:Update(tab, documents)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Updates", {
        tab = tab,
        list = documents
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param list table
---@return number
function MongoProxyComponent:Deletes(tab, list)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Deletes", {
        tab = tab,
        list = list
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param field string
---@param sort number
---@param unique boolean
---@return boolean
function MongoProxyComponent:SetIndex(tab, field, sort, unique)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code = node:Call(id, "MongoWriteProxy.SetIndex", {
        tab = tab,
        key = field,
        sort = sort or 1,
        unique = unique or false
    })
    return code == XCode.Ok
end

---@param tab string
---@param field string
---@param value number
---@return number
function MongoProxyComponent:Inc(tab, filter, field, value, upsert)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Inc", {
        tab = tab,
        field = field,
        filter = filter,
        upsert = upsert or false,
        value = value or 1
    })
    return code ~= XCode.Ok and 0 or response.value
end

---@param tab string
---@param filter table
---@param document table
function MongoProxyComponent:Save(tab, filter, document, upsert)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.Save", {
        tab = tab,
        filter = filter,
        document = document,
        upsert = upsert or false
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param filter table
---@param fields table
---@return table | nil
function MongoProxyComponent:FindOne(tab, filter, fields)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.FindOne", {
        tab = tab,
        filter = filter,
        fields = fields
    })
    return code == XCode.Ok and response or nil
end

---@param tab string
---@param filter table
---@param fields table
---@param limit number
---@return table | nil
function MongoProxyComponent:Find(tab, filter, fields, limit)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.Find", {
        tab = tab,
        filter = filter,
        fields = fields,
        limit = limit,
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list, response.cursor
end

function MongoProxyComponent:FindPage(tab, filter, fields, limit, page, sort)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.FindPage", {
        tab = tab,
        filter = filter,
        fields = fields,
        page = page or 1,
        limit = limit or 10,
        sort = sort
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

---@param tab string
---@param filter table
---@return number
function MongoProxyComponent:Count(tab, filter)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.Count", {
        tab = tab,
        filter = filter
    })
    if code ~= XCode.Ok then
        return 0
    end
    return response.count
end

---@param tab string
---@param cursor number
---@param count number
---@return table | nil
function MongoProxyComponent:GetMore(tab, cursor, count)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.GetMore", {
        tab = tab,
        cursor = cursor,
        batchSize = count
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

---@param tab string
---@param filter table
---@param count number
---@param fields table,
---@param batchSize number
---@return table | nil
function MongoProxyComponent:RandDocuments(tab, filter, count, fields, batchSize)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.Random", {
        tab = tab,
        filter = filter,
        count = count or 1,
        fields = fields,
        batchSize = batchSize
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

---@param tab string
---@param pipeline table
---@param batchSize number
---@return table | nil
function MongoProxyComponent:Aggregate(tab, pipeline, batchSize)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.Aggregate", {
        tab = tab,
        pipeline = pipeline,
        batchSize = batchSize
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

function MongoProxyComponent:ListDatabases()
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.ListDatabases")
    print(code)
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

function MongoProxyComponent:ListCollections(db)
    local id = node:Allot(MONGO_READ_PROXY)
    local code, response = node:Call(id, "MongoReadProxy.ListCollections", {
        db = db
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

---@param tab string
---@param filter table
---@param document table
---@param fields table
function MongoProxyComponent:FindModify(tab, filter, document, fields)
    local id = node:Allot(MONGO_WRITE_PROXY)
    local code, response = node:Call(id, "MongoWriteProxy.FindModify", {
        tab = tab,
        filter = filter,
        fields = fields,
        document = document
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response
end

return MongoProxyComponent