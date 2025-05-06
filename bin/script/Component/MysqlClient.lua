local MysqlClient = { }
local mysql = require("db.mysql")

local helper = require("SqlHelper")
function MysqlClient.Insert(name, tab)
    local sql = helper.InsertSql(name, tab)
    if sql == nil then
        return false
    end
    return mysql.run(sql)
end

function MysqlClient.Replace(name, tab)
    local sql = helper.ReplaceSql(name, tab)
    if sql == nil then
        return false
    end
    return mysql.run(sql)
end

function MysqlClient.Update(name, update, where)
    local sql = helper.UpdateSql(name, update, where)
    if sql == nil then
        return false
    end
    return mysql.run(sql)
end

function MysqlClient.Query(name, fields, where, limit)
    local sql = helper.QuerySql(name, fields, where, limit)
    if sql == nil then
        return nil
    end
    local code, response = mysql.run(sql)
    if not code then
        Log.Error("query sql = ", sql)
        return nil
    end
    return response
end

function MysqlClient.QueryOne(id, name, fields, where)
    local response = MysqlClient.Query(name, fields, where, 1)
    if response == nil then
        return nil
    end
    return response[1]
end

function MysqlClient.Delete(name, where)
    local sql = helper.DeleteSql(name, where)
    return mysql.run(sql)
end

return MysqlClient