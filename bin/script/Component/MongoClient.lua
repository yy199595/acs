
local mongo = require("db.mongo")
local json = require("util.json")
local json_encode = json.encode
local Component = require("Component")

local MongoClient = Component()

---@param tab string
---@param filter table
---@return number
function MongoClient:Count(tab, filter)
    local document = {
        filter = filter,
        ["$readPreference"] = {
            mode = "secondaryPreferred"
        },
    }
    local str = json_encode(document)
    local response = mongo.Run(tab, "count", str)
    if response and response.n then
        return response.n
    end
    return 0
end

---@param tab string
---@param filter table
---@param return table
function MongoClient:FindOnce(tab, filter)
    local document = {
        limit = 1,
        filter = filter,
        ["$readPreference"] = {
            mode = "secondaryPreferred"
        },
    }
    local str = json_encode(document)
    local response = mongo.Run(tab, "find", str)
    local documents = response.cursor.firstBatch
    if #documents == 0 then
        return nil
    end
    return documents[1]
end

---@param tab string
---@param filter table
---@param return table
function MongoClient:Find(tab, filter)
    local document = {
        filter = filter,
        ["$readPreference"] = {
            mode = "secondaryPreferred"
        },
    }
    local str = json_encode(document)
    local response = mongo.Run(tab, "find", str)
    return response.cursor.firstBatch
end

---@param tab string
---@param document table
---@return boolean
function MongoClient:InsertOnce(tab, document)
    local response = mongo.Run(tab, "insert", {
        documents = { document }
    })
    return response
end

---@param tab string
---@param filter table
---@return boolean
function MongoClient:DeleteOnce(tab, filter)
    local document = {
        limit = 1,
        q = filter
    }
    local response = mongo.Run(tab, "delete", { deletes = { document } })
    return response
end

---@param tab string
---@param filter table
function MongoClient:UpdateOnce(tab, filter, update, upsert)
    local document = {
        q = filter,
        multi = false,          --更新一个文档
        upsert = upsert or false, -- true不存在则插入
        u = {  ["$set"] = update }
    }
    local response = mongo.Run(tab, "update", { updates = { document } })
    return response and response.n == 1
end

---@param tab string
---@param filter table
---@param page number
---@param limit number
function MongoClient:FindPage(tab, filter, page, limit)

end


return MongoClient