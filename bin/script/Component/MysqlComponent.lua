
local app = require("App")
local json = require("util.json")

local Component = require("Component")

local MYSQL_DB = "MysqlDB"
local MYSQL_Exec = "MysqlDB.Exec"
local MYSQL_INDEX = "MysqlDB.Index"
local MYSQL_QUERY = "MysqlDB.Query"
local MYSQL_INSERT = "MysqlDB.Insert"
local MYSQL_UPDATE = "MysqlDB.Update"
local MYSQL_DELETE = "MysqlDB.Delete"
local MYSQL_FIND_PAGE = "MysqlDB.FindPage"


local json_encode = json.encode
local json_decode = json.decode
local table_insert = table.insert

local MysqlComponent = Component()

function MysqlComponent:GetActorId()
    if app:HasComponent(MYSQL_DB) then
        return nil
    end
    return app:Allot(MYSQL_DB)
end

---@param tab string
---@param data table
---@return number
function MysqlComponent:Insert(tab, data)
    local actorId = self:GetActorId()
    return app:Call(actorId, MYSQL_INSERT, {
        table = tab,
        document = json_encode(data)
    })
end

---@param tab string
---@param filter table
---@return number
function MysqlComponent:Delete(tab, filter, limit)
    local actorId = self:GetActorId()
    return app:Call(actorId, MYSQL_DELETE, {
        table = tab,
        limit = limit or 1,
        filter = json_encode(filter)
    })
end

---@param tab string
---@param filter table
---@param fields table
---@return table
function MysqlComponent:FindOne(tab, filter, fields)
    local request = {
        table = tab,
        limit = 1,
        filter = json_encode(filter)
    }
    if fields and next(fields) then
        request.fields = fields
    end
    local actorId = self:GetActorId()
    local code, results = app:Call(actorId, MYSQL_QUERY, request)
    if code ~= XCode.Ok or #results == 0 then
        return nil
    end
    return json_decode(results[1])
end

---@param tab string
---@param filter table
---@param fields table
---@param limit number
---@return number
function MysqlComponent:Find(tab, filter, fields, limit)
    local request = {
        table = tab,
        limit = limit or 0
    }
    if fields and next(fields) then
        request.fields = fields
    end
    if filter and next(filter) then
        request.filter = json_encode(filter)
    end
    local actorId = self:GetActorId()
    local code, response = app:Call(actorId, MYSQL_QUERY, request)
    if code ~= XCode.Ok or #response.documents == 0 then
        return nil
    end
    local documents = { }
    for _, str in ipairs(response.documents) do
        table_insert(documents, json_decode(str))
    end
    return documents
end

---@param tab string
---@param filter table
---@param update table
---@return number
function MysqlComponent:Update(tab, filter, update, limit)
    local actorId = self:GetActorId()
    return app:Call(actorId, MYSQL_UPDATE, {
        table = tab,
        limit = limit or 1,
        filter = json_encode(filter),
        document = json_encode(update)
    })
end

---@param tab string
---@param field string
---@param unique boolean
function MysqlComponent:CreateIndex(tab, field, unique)
    local actorId = self:GetActorId()
    return app:Call(actorId, MYSQL_INDEX, {
        tab = tab,
        name = field,
        unique = unique,
    })
end

---@param tab string
---@param filter table
---@param fields table
---@param page number
---@param count number
function MysqlComponent:FindPage(tab, filter, fields, page, count)
    local request = {
        table = tab,
        page = page or 1,
        limit = count or 10
    }
    if fields and next(fields) then
        request.fields = fields
    end
    if filter and next(filter) then
        request.filter = json_encode(filter)
    end
    local actorId = self:GetActorId()
    local code, response = app:Call(actorId, MYSQL_FIND_PAGE, request)
    if code ~= XCode.Ok or #response.documents == 0 then
        return nil
    end
    local documents = { }
    for _, str in ipairs(response.documents) do
        table_insert(documents, json_decode(str))
    end
    return documents
end

function MysqlComponent:Count(tab, filter)
    local sql = string.format("SELECT COUNT(*) FROM")
end

---@param sql string
function MysqlComponent:Exec(sql)
    local actorId = self:GetActorId()
    local code, response = app:Call(actorId, MYSQL_Exec, {
        sql = sql
    })
    if code ~= XCode.Ok or #response.documents == 0 then
        return nil
    end
    local documents = { }
    for _, str in ipairs(response.documents) do
        table_insert(jsons, json_decode(str))
    end
    return documents
end

return MysqlComponent