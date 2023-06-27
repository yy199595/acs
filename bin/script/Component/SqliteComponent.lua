
local this = _G.Sqlite
local json = _G.rapidjson
local log_err = require("Log").OnError
local SqliteComponent = Class("Component")

SqliteComponent.databases = { }
local sql_helper = require("SqlHelper")

local GetDbInfo = function(tab)
    local result = string.split(tab, ".")
    return result[1], result[2]
end

function SqliteComponent:FindIdByName(db)
    local id = self.databases[db]
    if id == nil then
        id = this.Open(db)
        self.databases[db] = id
    end
    return id
end

function SqliteComponent:Drop(name)
    local func = this.Exec
    local db, tab = GetDbInfo(name)
    local id = self:FindIdByName(db)
    local sql = string.format("DROP TABLE %s;", tab)
    local status, response = xpcall(func, log_err, id, sql)
    if not status then
        return false
    end
    return response
end

function SqliteComponent:Create(name, data, index)
    local func = this.Exec
    local db, tab = GetDbInfo(name)
    local id = self:FindIdByName(db)
    local sql = sql_helper.CreateSql(tab, data)
    local status, response = xpcall(func, log_err, id, sql)
    if not status then
        return
    end
    return response
end

function SqliteComponent:Insert(name, data)

    local func = this.Exec
    local db, tab = GetDbInfo(name)
    local id = self:FindIdByName(db)
    local sql = sql_helper.InsertSql(tab, data)
    local status, response = xpcall(func, log_err, id, sql)
    if not status then
        return
    end
    return response
end

function SqliteComponent:FindOne(name, select, fields)
    local func = this.Query
    local db, tab = GetDbInfo(name)
    local id = self:FindIdByName(db)
    local sql = sql_helper.QuerySql(tab, fields, select, 1)
    print(sql)
    local status, response = xpcall(func, log_err, id, sql)
    if not status then
        return
    end
    return response[1]
end

return SqliteComponent