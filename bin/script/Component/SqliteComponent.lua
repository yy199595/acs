require("StringUtil")

local helper = require("SqlHelper")
local sqlite = require("db.sqlite")

local string_format = string.format

local SqliteComponent = { }

function SqliteComponent:Drop(name)
    return sqlite.exec(string_format("DROP TABLE %s;", name))
end

function SqliteComponent:Create(name, data, index)
    local sql = helper.CreateSql(name, data, index)
    return sqlite.exec(sql)
end

function SqliteComponent:UpdateOne(tab, filter, document)
    return sqlite.exec(helper.UpdateSql(tab, document, filter))
end

function SqliteComponent:SetIndex(tab, field, unique)
    return sqlite.exec(helper.SetIndex(tab, field, unique))
end

function SqliteComponent:CreateTabAutoKey(name, key)
    return sqlite.exec(string_format("CREATE TABLE %s (%s INTEGER PRIMARY KEY AUTOINCREMENT);", name, key))
end

function SqliteComponent:SetAutoKey(name, key)
    return sqlite.exec(string_format("ALTER TABLE %s MODIFY %s INT AUTO_INCREMENT;", name, key))
end

function SqliteComponent:AddPrimaryKey(name, key)
    return sqlite.Exec(helper.AddPrimaryKeySql(name, key))
end

function SqliteComponent:Insert(name, data)
    return sqlite.exec(helper.InsertSql(name, data))
end

function SqliteComponent:IsExist(tab)
    local sql = string_format("SELECT name FROM sqlite_master WHERE type='table' AND name='%s';", tab)
    local response = sqlite.query(sql)
    return response ~= nil and #response > 0
end

function SqliteComponent:Query(tab, filter)
    return sqlite.query(helper.QuerySql(tab, nil, filter))
end

function SqliteComponent:FindOne(name, filter, fields)
    local response = sqlite.query(helper.QuerySql(name, fields, filter, 1))
    if response == nil or not next(response) then
        return nil
    end
    return response[1]
end

return SqliteComponent