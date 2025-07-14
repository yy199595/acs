require("StringUtil")

local xpcall = xpcall
local sqlite = require("db.sqlite")
local factory = require("SqlFactory")
local string_format = string.format
local log_error = require("Log").OnError

local SqliteComponent = { }

local sqlite_run = sqlite.run
local sqlite_get = sqlite.get
local sqlite_set = sqlite.set
local sqlite_del = sqlite.del
local sqlite_build = sqlite.build
local sqlite_invoke = sqlite.invoke
local sqlite_timeout = sqlite.set_timeout

function SqliteComponent:Drop(name)
    return sqlite_run(string_format("DROP TABLE %s;", name))
end

function SqliteComponent:UpdateOne(tab, filter, document)
    local sql = factory:GetTable(tab):Update(document):Filter(filter):ToString()
    local response = sqlite_run(sql)
    if not response.ok then
        return nil
    end
    return response.list
end


function SqliteComponent:Count(tab, filter)
    local sql = factory:GetTable(tab):Count():Filter(filter):ToString()
    local response = sqlite_run(sql)
    if not response.ok then
        return nil
    end
    return response.list[1].count
end

function SqliteComponent:InsertOne(name, document)
    local sql = factory:GetTable(name):Insert(document):ToString()
    local response = sqlite_run(sql)
    if response == nil or not response.ok then
        return 0
    end
    return response.ok
end

function SqliteComponent:InsertBatch(name, documents)
    local count = 0
    for _, document in ipairs(documents) do
        local sql = factory:GetTable(name):Insert(document):ToString()
        local ok, response = xpcall(sqlite_run, log_error, sql)
        if ok and response.ok and response.count > 0 then
            count = count + response.count
        end
    end
    return count
end

function SqliteComponent:Run(sql)
    local response = sqlite_run(sql)
    if not response.ok then
        return nil
    end
    return response.list
end

function SqliteComponent:Query(tab, filter, fields)
    local sql = factory:GetTable(tab):Select(fields):Filter(filter):ToString()
    local response = sqlite_run(sql)
    if not response.ok then
        return nil
    end
    return response.list
end

function SqliteComponent:FindOne(name, filter, fields)
    local sql = factory:GetTable(name):Select(fields):Filter(filter):Limit(1):ToString()
    local response = sqlite_run(sql)
    if not response.ok or response.list == nil then
        return nil
    end
    return response.list[1]
end

function SqliteComponent:Find(name, filter, sort, fields, limit)
    local sql = factory:GetTable(name):Select(fields):Filter(filter):OrderBy(sort):Limit(limit):ToString()
    local response = sqlite_run(sql)
    if not response.ok or response.list == nil then
        return nil
    end
    return response.list
end

function SqliteComponent:TableInfo(tab)
    local sql = string_format("PRAGMA table_info(%s)", tab)
    return self:Run(sql)
end

---@param name string
---@param sql string
function SqliteComponent:Build(name, sql)
    return sqlite_build(name, sql)
end

---@param name string
function SqliteComponent:Invoke(name, ...)
    return sqlite_invoke(name, ...)
end

---@param key string
---@param value string | table
function SqliteComponent:Set(key, value)
    return sqlite_set(key, value)
end

---@param key string
---@return table | string
function SqliteComponent:Get(key)
    return sqlite_get(key)
end

---@param key string
---@param second number
function SqliteComponent:SetTimeout(key, second)
    return sqlite_timeout(key, second)
end

---@param key string
---@return boolean
function SqliteComponent:Delete(key)
    return sqlite_del(key)
end

return SqliteComponent