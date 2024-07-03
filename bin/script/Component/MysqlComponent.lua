
local app = require("App")
local json = require("util.json")

local Component = require("Component")

local MYSQL_DB = "MysqlDB"
local MYSQL_INSERT = "MysqlDB.Add"
local MYSQL_QUERY = "MysqlDB.Query"
local MYSQL_UPDATE = "MysqlDB.Update"
local MYSQL_DELETE = "MysqlDB.Delete"


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
        data = json_encode(data)
    })
end

---@param tab string
---@param filter table
---@return number
function MysqlComponent:Delete(tab, filter)
    local actorId = self:GetActorId()
    return app:Call(actorId, MYSQL_DELETE, {
        table = tab,
        where_json = json_encode(filter)
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
        where_json = json_encode(filter)
    }
    if fields and next(fields) then
        request.fields = fields
    end
    local actorId = self:GetActorId()
    local code, results = app:Call(actorId, MYSQL_QUERY, request)
    if code ~= XCode.Ok or #results == 0 then
        return nil
    end
    local str = results[1]
    return json_decode(str)
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
        request.where_json = json_encode(filter)
    end
    local actorId = self:GetActorId()
    local code, response = app:Call(actorId, MYSQL_QUERY, request)
    if code ~= XCode.Ok or #response.jsons == 0 then
        return nil
    end
    local jsons = { }
    for _, str in ipairs(response.jsons) do
        table_insert(jsons, json_decode(str))
    end
    return jsons
end

---@param tab string
---@param filter table
---@param update table
---@return number
function MysqlComponent:Update(tab, filter, update)
    local actorId = self:GetActorId()
    return app:Call(actorId, MYSQL_UPDATE, {
        table = tab,
        where_json = json_encode(filter),
        update_json = json_encode(update)
    })
end

return MysqlComponent