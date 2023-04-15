local MysqlClient = { }
local mysqlCli = Mysql
local helper = require("Server.SqlHelper")
function MysqlClient.Insert(id, name, tab)
    local sql = helper.InsertSql(name, tab)
    if sql == nil then
        return false
    end
    return mysqlCli.Exec(id, sql)
end

function MysqlClient.Replace(id, name, tab)
    local sql = helper.ReplaceSql(name, tab)
    if sql == nil then
        return false
    end
    return mysqlCli.Exec(id, sql)
end

function MysqlClient.Update(id, name, update, where)
    local sql = helper.UpdateSql(name, update, where)
    if sql == nil then
        return false
    end
    return mysqlCli.Exec(id, sql)
end

function MysqlClient.Query(id, name, fields, where, limit)
    local sql = helper.QuerySql(name, fields, where, limit)
    if sql == nil then
        return nil
    end
    return mysqlCli.Query(id, sql)
end

function MysqlClient.NewTable(id, tab, tableInfo)
    local pb = tableInfo.pb
    local keys = tableInfo.keys
    local fields = tableInfo.fields
    assert(type(pb) == "string")
    assert(type(keys) == "table")
    assert(type(fields) == "table")
    return mysqlCli.CreateTable(id, tab, pb, fields, keys)
end

return MysqlClient