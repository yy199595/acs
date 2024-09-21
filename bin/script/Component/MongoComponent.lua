local type = _G.type
local ipairs = _G.ipairs
local assert = _G.assert
local log = require("Log")
local app = require("App")
local json = require("util.json")
local Component = require("Component")

local MONGO_DB = "MongoDB"
local log_error = log.Error
local json_encode = json.encode
local json_decode = json.decode
local table_insert = table.insert

local MongoComponent = Component()

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
        document = json.encode(data)
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Insert", {
        tab = tab,
        jsons = { document },
    })
end

function MongoComponent:Insert(tab, documents)
    local jsons = { }
    for _, document in ipairs(documents) do
        table_insert(jsons, json_encode(document))
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Insert", {
        tab = tab,
        jsons = jsons
    })
end

function MongoComponent:Save(tab, select, data)
    if tab == nil or data == nil then
        log_error("tab=%s data=%s", tab, data)
        return
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Save", {
        tab = tab,
        select = json_encode(select),
        update = json_encode(data),
        tag = "$set",
    })
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
        json = json_encode(data)
    })
end

---@param tab string
---@param filter table|string|number
---@return table
function MongoComponent:FindOne(tab, filter, fields)

    local request = {
        tab = tab,
        limit = 1,
        fields = fields or { }
    }
    if type(filter) ~= "table" then
        request.where = json_encode({ _id = filter })
    elseif filter and next(filter) then
        request.where = json_encode(filter)
    end
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.FindOne", request)

    if code ~= XCode.Ok then
        log_error("query from [%s] where => %s code:%s", tab, json.encode(data), code)
        return nil
    end
    return json_decode(response.json)
end

---@param tab string
---@param select table
---@param page number
---@param count number
---@param files table
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
        request.where = json_encode(select)
    end

    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.FindPage", request)
    if code ~= XCode.Ok then
        log_error("query from %s where:%s code:%s", tab, request.where, code)
        return nil
    end
    local result = { }
    for _, str in ipairs(response.json) do
        table_insert(result, json_decode(str))
    end
    return result
end

---@param tab string
---@param where table
---@param limit number
---@return table
function MongoComponent:Find(tab, where, fields, limit)

    local request = {
        tab = tab,
        limit = limit or 0,
    }
    if where then
        request.json = json_encode(where)
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
    for _, str in ipairs(response.jsons) do
        table_insert(responses, json_decode(str))
    end
    return responses
end
-- 同时查询多个 _id,参数是匹配的_id列表
---@param tab string
---@param wheres table
---@param fields table
---@return table
function MongoComponent:FindWhere(tab, wheres, fields, field)
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
        json = json_encode(filter)
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
    for _, str in ipairs(response.jsons) do
        table_insert(responses, json_decode(str))
    end
    return responses
end

---@param tab string
---@param name string
---@param unique boolean
---@return number
function MongoComponent:SetIndex(tab, name, unique)
    assert(type(tab) == "string")
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.SetIndex", {
        tab = tab,
        key = name,
        unique = unique or false
    })
end

---@param tab string
---@param select table|string
---@param update table
---@param tag string
---@return number
function MongoComponent:Update(tab, select, update, tag, upsert)
    if select == nil or update == nil then
        log_error("select=%s update=%s", select, update)
        return
    end
    local request = {
        tab = tab,
        tag = tag or "$set",
        document = {
            filter = json_encode(select),
            document = json_encode(update)
        },
        upsert = upsert or false,
    }
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Update", request)
end

function MongoComponent:Updates(tab, documents, tag)
    local request = {
        tab = tab,
        tag = tag or "$set",
        document = { }
    }
    for _, value in ipairs(documents) do
        local filter = json_encode(value.filter)
        local updater = json_encode(value.document)
        table_insert(request.document, {
            filter = filter,
            document = updater
        })
    end
    local session = self:GetActorId()
    return app:Call(session, "MongoDB.Updates", request)
end

function MongoComponent:UpdateById(tab, id, update, tag)
    local filter = { _id = id }
    return self:Update(tab, filter, update, tag)
end

---@param tab string
---@param select table
---@param field string
---@param value any
---@return number
function MongoComponent:Push(tab, select, field, value)
    return self:Update(tab, select, {
        [field] = value
    }, "$push")
end

---@param tab string
---@return number
function MongoComponent:Drop(tab)
    local result = self:RunCommand(tab, "drop")
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
        request.where = json_encode(select)
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
function MongoComponent:RunCommand(tab, cmd, document)
    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Command", {
        tab = tab,
        cmd = cmd,
        json = document and json_encode(document) or ""
    })
    if code ~= XCode.Ok then
        return
    end
    return json_decode(response.json)
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
    return self:Update(tab, filter, {
        [field] = value
    }, "$addToSet", upsert or false)
end

---@param tab string
---@param filter table
---@param field string
---@return number
function MongoComponent:DelArray(tab, filter, field, value)
    return self:Update(tab, filter, {
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
    return json_decode(response.json)
end

function MongoComponent:Sum(tab, filter, by, field)

    local request = {
        by = by,
        tab = tab,
        field = field
    }
    if filter and next(filter) then
        request.filter = json_encode(filter)
    end

    local session = self:GetActorId()
    local code, response = app:Call(session, "MongoDB.Sum", request)
    if code ~= XCode.Ok then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.json) do
        table_insert(responses, json_decode(str))
    end
    return responses
end

return MongoComponent