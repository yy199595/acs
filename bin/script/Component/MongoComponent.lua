local type = _G.type
local ipairs = _G.ipairs
local assert = _G.assert
local log = require("Log")
local app = require("App")
local json = require("util.json")

local MONGO_DB = "MongoDB"
local log_error = log.Error
local json_encode = json.encode
local json_decode = json.decode
local table_insert = table.insert

local MongoComponent = { }

function MongoComponent:GetActorId()
    if app:HasComponent(MONGO_DB) then
        return nil
    end
    return app:Allot(MONGO_DB)
end

---@param tab string
---@param data table
---@return number
function MongoComponent:InsertOnce(tab, data)
    local document = data
    if type(data) == "table" then
        document = json_encode(data)
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Insert", {
        tab = tab,
        documents = { document },
    })
end

function MongoComponent:Insert(tab, documents)
    local results = { }
    for _, document in ipairs(documents) do
        table_insert(results, json_encode(document))
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Insert", {
        tab = tab,
        documents = results
    })
end

function MongoComponent:Save(tab, filter, document)
    return self:UpdateOne(tab, filter, document, "$set", true)
end

---@param tab string
---@param data table
---@param limit number
---@return number
function MongoComponent:Delete(tab, data, limit)
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Delete", {
        tab = tab,
        limit = limit or 1,
        filter = json_encode(data)
    })
end

---@param tab string
---@param filter table|string|number
---@return table
function MongoComponent:FindOne(tab, filter, fields)

    if tab == nil then
        return XCode.CallArgsError
    end
    local request = {
        tab = tab,
        limit = 1,
        fields = fields or { }
    }

    if type(filter) ~= "table" then
        request.filter = json_encode({ _id = filter })
    elseif filter and next(filter) then
        request.filter = json_encode(filter)
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.FindOne", request)

    if code ~= XCode.Ok then
        log_error("query from [{}] filter => {} code:{}", tab, filter, code)
        return nil
    end
    return json_decode(response.document)
end

---@param tab string
---@param select table
---@param page number
---@param count number
---@param fields table
---@return table
function MongoComponent:FindPage(tab, select, page, count, fields, sort)
    local request = {
        tab = tab,
        page = page,
        count = count or 10,
    }

    if fields and next(fields) then
        request.fields = fields
    end

    if sort and next(sort) then
        request.sort = json_encode(sort)
    end

    if select and type(select) == "table" then
        request.filter = json_encode(select)
    end

    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.FindPage", request)
    if code ~= XCode.Ok then
        log_error("query from %s filter:%s code:%s", tab, request.filter, code)
        return nil
    end
    local result = { }
    for _, str in ipairs(response.documents) do
        table_insert(result, json_decode(str))
    end
    return result
end

---@param tab string
---@param filter table
---@param limit number
---@return table
function MongoComponent:Find(tab, filter, fields, limit)

    local request = {
        tab = tab,
        limit = limit or 0,
    }
    if filter then
        request.filter = json_encode(filter)
    end
    if fields and next(fields) then
        request.fields = fields
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Find", request)
    if code ~= XCode.Ok or response == nil then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.documents) do
        table_insert(responses, json_decode(str))
    end
    return responses, response.cursor
end
-- 同时查询多个 _id,参数是匹配的_id列表
---@param tab string
---@param wheres table
---@param fields table
---@return table
function MongoComponent:FindIn(tab, wheres, fields, field)
    local count = #wheres
    assert(type(tab) == "string")
    assert(type(wheres) == "table" and count > 0)

    local filter = {
        [field or "_id"] = {
            ["$in"] = wheres
        }
    }
    local request = {
        tab = tab,
        limit = count,
        filter = json_encode(filter)
    }
    if fields and next(fields) then
        request.fields = fields
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Find", request)
    if code ~= XCode.Ok or response == nil then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.documents) do
        table_insert(responses, json_decode(str))
    end
    return responses
end

---@param tab string
---@param name string
---@param unique boolean
---@param sort number
---@return number
function MongoComponent:SetIndex(tab, name, sort, unique)
    assert(type(tab) == "string")
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.SetIndex", {
        tab = tab,
        key = name,
        sort = sort or 1,
        unique = unique or false
    })
end

---@param tab string
---@param select table|string
---@param update table
---@param tag string
---@return number
function MongoComponent:Update(tab, filter, update, cmd, upsert)
    if filter == nil or update == nil then
        log_error("select={} update={}", filter, update)
        return
    end
    local request = {
        tab = tab,
        document = {
            cmd = cmd or "$set",
            filter = json_encode(filter),
            document = json_encode(update)
        },
        multi = true,
        upsert = upsert or false,
    }
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Update", request)
end

---@param tab string
---@param select table|string
---@param update table
---@param tag string
---@return number
function MongoComponent:UpdateOne(tab, filter, update, cmd, upsert)
    if filter == nil or update == nil then
        log_error("select={} update={}", filter, update)
        return
    end
    local request = {
        tab = tab,
        document = {
            cmd = cmd or "$set",
            filter = json_encode(filter),
            document = json_encode(update)
        },
        multi = false,
        upsert = upsert or false,
    }
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Update", request)
end

function MongoComponent:Updates(tab, documents, cmd)
    local request = {
        tab = tab,
        document = { }
    }
    cmd = cmd or "$set"
    for _, value in ipairs(documents) do
        local filter = json_encode(value.filter)
        local updater = json_encode(value.document)
        table_insert(request.document, {
            filter = filter,
            document = updater,
            cmd = value.cmd or cmd
        })
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Updates", request)
end

function MongoComponent:UpdateById(tab, id, update, tag)
    local filter = { _id = id }
    return self:UpdateOne(tab, filter, update, tag)
end

---@param tab string
---@param select table
---@param field string
---@param value any
---@return number
function MongoComponent:Push(tab, select, field, value)
    return self:UpdateOne(tab, select, {
        [field] = value
    }, "$push")
end

---@param tab string
---@return number
function MongoComponent:Drop(tab)
    local result = self:Run(tab, "drop")
    return result == nil and XCode.Failure or XCode.Ok
end

---@param tab string
---@param index string
---@return number
function MongoComponent:DropIndex(tab, index)
    return self:Command(tab, "dropIndexes", {
        index = index
    })
end

---@param tab string
---@param select table
---@return number
function MongoComponent:Count(tab, select)
    local request = { tab = tab }
    if select ~= nil then
        request.filter = json_encode(select)
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Count", request)
    if code ~= XCode.Ok then
        log.Error("tab: %s select: %s", tab, json_encode(select))
    end
    return code == XCode.Ok and response.count or 0
end

---@param tab string
---@param document table
---@return number,table
function MongoComponent:Run(tab, cmd, document)
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Run", {
        tab = tab,
        cmd = cmd,
        document = document and json_encode(document) or ""
    })
    if code ~= XCode.Ok then
        return
    end
    return json_decode(response.document)
end

---@param key string
---@return number
function MongoComponent:Inc(key)
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Inc", {
        key = key
    })
    if code ~= XCode.Ok or response == nil then
        return nil
    end
    return response.value
end

---@param tab string
---@param filter table
---@param field string
---@param value any
---@param upsert boolean
---@return number
function MongoComponent:PushSet(tab, filter, field, value, upsert)
    return self:UpdateOne(tab, filter, {
        [field] = value
    }, "$addToSet", upsert or false)
end

---@param tab string
---@param filter table
---@param field string
---@return number
function MongoComponent:DelArray(tab, filter, field, value)
    return self:UpdateOne(tab, filter, {
        [field] = value
    }, "$pull", false)
end

function MongoComponent:Databases()
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Databases")
    if code ~= XCode.Ok or response == nil then
        return nil
    end
    return response.array
end

function MongoComponent:Collections(name)
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Collections", {
        str = name
    })
    if code ~= XCode.Ok or response == nil then
        return nil
    end
    return response.array
end

function MongoComponent:FindAndModify(tab, query, update, fields)
    if query == nil or update == nil then
        log_error("query=%s update=%s", query, update)
        return
    end
    local request = {
        tab = tab
    }
    if query and next(query) then
        request.query = json_encode(query)
    end

    if update and next(update) then
        request.update = json_encode(update)
    end

    if fields and next(fields) then
        request.fields = fields
    end

    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.FindAndModify", request)
    if code ~= XCode.Ok then
        return nil
    end
    return json_decode(response.document)
end

---@param tab string
---@param cursor number
---@param batchSize number
function MongoComponent:GetMore(tab, cursor, batchSize)
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.GetMore", {
        tab = tab,
        cursor = cursor,
        batchSize = batchSize
    })
    if code ~= XCode.Ok then
        return nil
    end
    local documents = { }
    for _, str in ipairs(response.documents) do
        table_insert(documents, json_decode(str))
    end
    return documents, response.cursor
end

function MongoComponent:Facet(tab, _id, filters, group, batchSize)
    local request = {
        tab = tab,
        _id = _id,
        match = { },
        batchSize = batchSize
    }
    --if _id == nil then
    --    return XCode.CallArgsError
    --end
    if group == nil or not next(group) then
        return XCode.CallArgsError
    end
    if filters == nil or not next(filters) then
        return XCode.CallArgsError
    end
    request.group = json_encode(group)
    for _, filter in ipairs(filters) do
        table_insert(request.match, json_encode(filter))
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Facet", request)
    if code ~= XCode.Ok then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.documents) do
        table_insert(responses, json_decode(str))
    end
    return responses, response.cursor
end

function MongoComponent:Aggregate(tab, cmd, field, filter, by, batchSize)

    local request = {
        by = by,
        tab = tab,
        cmd = cmd,
        field = field,
        batchSize = batchSize
    }
    if filter and next(filter) then
        request.filter = json_encode(filter)
    end

    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Aggregate", request)
    if code ~= XCode.Ok then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.documents) do
        table_insert(responses, json_decode(str))
    end
    return responses, response.cursor
end

function MongoComponent:Distinct(tab, key, filter)
    local request = {
        tab = tab,
        key = key
    }
    if filter and next(filter) then
        request.filter = json_encode(filter)
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Distinct", request)
    if code ~= XCode.Ok or not response then
        return nil
    end
    local result = json_decode(response.document)
    if not result then
        return nil
    end
    return result.values
end


return MongoComponent