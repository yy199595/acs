
require("StringUtil")
local app = require("App")
local helper = require("SqlHelper")
local sqlite = require("db.sqlite")

local table_unpack = table.unpack
local string_split = string.split
local string_format = string.format

local SqliteComponent = { }

local GetDbInfo = function(tab)
    local res = string_split(tab, ".")
    return table_unpack(res)
end

function SqliteComponent:Drop(name)
    local db, tab = GetDbInfo(name)
    return sqlite.Exec(db, string_format("DROP TABLE %s;", tab))
end

function SqliteComponent:Create(name, data, index)
    local db, tab = GetDbInfo(name)
    local sql = helper.CreateSql(tab, data, index)
    return sqlite.Exec(db, sql)
end

function SqliteComponent:CreateTabAutoKey(name, key)
    local db, tab = GetDbInfo(name)
    return sqlite.Exec(db, string_format("CREATE TABLE %s (%s INTEGER PRIMARY KEY AUTOINCREMENT);", tab, key))
end

function SqliteComponent:AddField(name, key, value)

    local db, tab = GetDbInfo(name)
    return sqlite.Exec(db, helper.AddFieldSql(tab, key, value))
end

function SqliteComponent:SetAutoKey(name, key)

    local db, tab = GetDbInfo(name)
    return sqlite.Exec(db, string_format("ALTER TABLE %s MODIFY %s INT AUTO_INCREMENT;", tab, key))
end

function SqliteComponent:AddPrimaryKey(name, key)

    local db, tab = GetDbInfo(name)
    return sqlite.Exec(db, helper.AddPrimaryKeySql(tab, key))
end

function SqliteComponent:Insert(name, data)

    local db, tab = GetDbInfo(name)
    return sqlite.Exec(db, helper.InsertSql(tab, data))
end

function SqliteComponent:FindOne(name, select, fields)
    local db, tab = GetDbInfo(name)
    return sqlite.FindOne(db, helper.QuerySql(tab, fields, select, 1))
end

return SqliteComponent