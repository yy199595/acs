local PgsqlProxyComponent = { }

local node = require("Node")
local table_insert = table.insert
local table_concat = table.concat
local string_format = string.format
local PGSQL_READ_PROXY = "PgsqlReadProxy"
local PGSQL_WRITE_PROXY = "PgsqlWriteProxy"

---@param tab string
---@param document table
---@return boolean
function PgsqlProxyComponent:InsertOne(tab, document)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.Insert", {
        tab = tab,
        document = document
    })
    return code == XCode.Ok and response.count or 0
end

---@param tab string
---@param documents table
---@return number
function PgsqlProxyComponent:InsertBatch(tab, documents)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.InsertBatch", {
        tab = tab,
        documents = documents
    })
    if code ~= XCode.Ok then
        return 0
    end
    return response.count
end

---@param stmt string
function PgsqlProxyComponent:ExecuteInWrite(stmt, ...)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.Execute", {
        stmt = stmt,
        args = { ... }
    })
    return code == XCode.Ok and response.count or 0

end

---@param stmt string
function PgsqlProxyComponent:ExecuteInRead(stmt, ...)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.Execute", {
        stmt = stmt,
        args = { ... }
    })
    return code == XCode.Ok and response.list or nil
end

---@param tab string
---@param document table
---@return boolean
function PgsqlProxyComponent:ReplaceOne(tab, document)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code = node:Call(id, "PgsqlWriteProxy.Replace", {
        tab = tab,
        document = document
    })
    return code == XCode.Ok
end

---@param tab string
---@param filter table | string
---@return boolean
function PgsqlProxyComponent:Delete(tab, filter, limit)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code = node:Call(id, "PgsqlWriteProxy.Delete", {
        tab = tab,
        limit = limit or 1,
        filter = filter
    })
    return code == XCode.Ok
end

---@param tab string
---@param filter table | string
---@param field string
---@field value number
---@return number
function PgsqlProxyComponent:Inc(tab, filter, field, value)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.Inc", {
        tab = tab,
        field = field,
        filter = filter,
        value = value or 1
    })
    return code == XCode.Ok and response.value or 0
end

---@param tab string
---@param filter table | string
---@param document table
---@return boolean
function PgsqlProxyComponent:Update(tab, filter, document)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.Update", {
        tab = tab,
        filter = filter,
        document = document
    })
    return code == XCode.Ok and response.count or 0
end

---@param documents table
---@return number
function PgsqlProxyComponent:UpdateBatch(documents)
    assert(#documents > 0)
    for _, document in ipairs(documents) do
        assert(document.tab)
        assert(document.filter)
        assert(document.document)
    end
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.UpdateBatch", documents)
    return code == XCode.Ok and response.count or 0
end

---@param tab string
---@param field string
---@param unique boolean
---@return boolean
function PgsqlProxyComponent:SetIndex(tab, field, unique)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code = node:Call(id, "PgsqlWriteProxy.SetIndex", {
        tab = tab,
        field = field,
        unique = unique or false
    })
    return code == XCode.Ok
end

---@param sql string
function PgsqlProxyComponent:RunInWrite(sql)
    local id = node:Allot(PGSQL_WRITE_PROXY)
    local code, response = node:Call(id, "PgsqlWriteProxy.Run", {
        sql = sql
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

---@param sql string
function PgsqlProxyComponent:RunInRead(sql)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.Run", sql)
    if code ~= XCode.Ok then
        return nil
    end
    return response.list
end

---@param tab string
---@param filter table | string
---@param fields table
---@return table | nil
function PgsqlProxyComponent:FindOne(tab, filter, fields)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.Find", {
        tab = tab,
        limit = 1,
        filter = filter,
        fields = fields,
    })
    return code ~= XCode.Ok and nil or response
end

---@param tab string
---@param filter table
---@return number
function PgsqlProxyComponent:Count(tab, filter)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.Count", {
        tab = tab,
        filter = filter,
    })
    return code ~= XCode.Ok and 0 or response.count
end

---@param tab string
---@param filter table | string
---@param fields table
---@return table | nil
function PgsqlProxyComponent:Find(tab, filter, fields, sort, limit)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.Find", {
        tab = tab,
        sort = sort,
        limit = limit or 0,
        filter = filter,
        fields = fields,
    })
    return code ~= XCode.Ok and nil or response
end

---@param tab string
---@param field string
---@param list table
---@param fields table | nil
---@param sort table | nil
function PgsqlProxyComponent:FindIn(tab, field, list, fields, sort)
    return PgsqlProxyComponent:Find(tab, { [field] = list }, sort, fields, #list)
end

---@param tab string
---@param filter table | string
---@param fields table
---@param page number
---@param count number
---@return table | nil
function PgsqlProxyComponent:FindPage(tab, filter, sort, fields, page, count)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.FindPage", {
        tab = tab,
        sort = sort,
        filter = filter,
        fields = fields,
        page = page or 1,
        count = count or 10
    })
    if code ~= XCode.Ok then
        return nil
    end
    return response
end

---@param tab string
---@param func string
---@param field string
---@param filter table
---@return table
function PgsqlProxyComponent:Func(tab, func, field, filter, group)
    local id = node:Allot(PGSQL_READ_PROXY)
    local code, response = node:Call(id, "PgsqlReadProxy.Func", {
        tab = tab,
        func = func,
        field = field,
        filter = filter,
        group =group,
    })
    return code ~= XCode.Ok and nil or response
end

return PgsqlProxyComponent