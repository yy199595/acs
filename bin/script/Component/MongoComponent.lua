
local type = _G.type
local ipairs = _G.ipairs
local assert = _G.assert
local json = require("json")
local table_insert = table.insert
local string_format = string.format
local log_error = require("Log").Error
local MongoComponent = Class("Component")

---@param tab string
---@param data table
---@param flag number
---@returns number
function MongoComponent:InsertOnce(tab, data)
    local document = json.encode(data)
    local session = self.app:Allot("MongoDB")
    return self.app:Call(session, "MongoDB.Insert", {
        tab = tab,
        json = document,
    })
end

---@param tab string
---@param data table
---@param limit number
---@param flag number
---@return number
function MongoComponent:Delete(tab, data, limit)
    local session = self.app:Allot("MongoDB")
    return self.app:Call(session,"MongoDB.Delete", {
        tab = tab,
        limit = limit or 1,
        json = json.encode(data)
    })
end

---@param tab string
---@param data table
---@return table
function MongoComponent:FindOne(tab, data, files)
    local session = self.app:Allot("MongoDB")
    local code, response = self.app:Call(session, "MongoDB.FindOne", {
        tab = tab,
        limit = 1,
        where = json.encode(data),
        files = files or { }
    })
    if code ~= XCode.Successful or response == nil then
        log_error(string_format("query from %s where = %s", tab, json.encode(data)))
        return nil
    end
    return json.decode(response.json)
end

---@param tab string
---@param where table
---@param limit number
---@return table
function MongoComponent:Find(tab, where, limit)

    local session = self.app:Allot("MongoDB")
    local code, response = self.app:Call(session, "MongoDB.Find", {
        tab = tab,
        limit = limit or 0,
        json = json.encode(where)
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.jsons) do
        table_insert(responses, json.decode(str))
    end
    return responses
end
-- 同时查询多个 _id,参数是匹配的_id列表
---@param tab string
---@param wheres table
---@return table
function MongoComponent:FindWhere(tab, wheres)
    local count = #wheres
    assert(type(tab) == "string")
    assert(type(wheres) == "table" and count > 0)

    local request = {
        _id = {
            ["$in"] = wheres
        }
    }
    local session = self.app:Allot("MongoDB")
    local code, response = self.app:Call(session, "MongoDB.Find", {
        tab = tab,
        limit = count,
        json = json.encode(request)
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    local responses = {}
    for _, str in ipairs(response.jsons) do
        table_insert(responses, json.decode(str))
    end
    return responses
end

---@param tab string
---@param name string
---@return number
function MongoComponent:SetIndex(tab, keys)
    assert(type(tab) == "string")
    local session = self.app:Allot("MongoDB")
    return self.app:Call(session, "MongoDB.SetIndex", {
        tab = tab,
        keys = keys
    })
end

---@param tab string
---@param select table
---@param update table
---@param flag number
---@param tag string
---@return number
function MongoComponent:Update(tab, select, update, tag)
    assert(type(select) == "string")
    assert(type(update) == "string")
    local session = self.app:Allot("MongoDB")
    return self.app:Call(session, "MongoDB.Update", {
        tab = tab,
        select = json.encode(select),
        update = json.encode(update),
        tag = tag or "$set",
    })
end

---@param tab string
---@param select table
---@param update table
---@param flag number
---@return number
function MongoComponent:Push(tab, select, update)
    return self:Update(tab, select, update,"$push")
end

---@param tab string
---@return number
function MongoComponent:Drop(tab)
    return self:Delete(tab,{ }, 0)
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
---@param document table
---@param flag number
---@return number,table
function MongoComponent:Command(tab, cmd, document)
    local session = self.app:Allot("MongoDB")
    local code, response = self.app:Call(session, "MongoDB.Command", {
        tab = tab,
        cmd = cmd,
        json = document and json.encode(document) or ""
    })
    if code ~= XCode.Successful then
        return
    end
    return json.decode(response.json)
end

return MongoComponent