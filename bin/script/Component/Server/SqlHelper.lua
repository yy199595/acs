local SqlHelper = {}

function SqlHelper.InsertSql(name, tab)
    assert(type(name) == "string")
    assert(type(tab) == "table")
    local keys = { }
    local values = { }
    for i, v in pairs(tab) do
        table.insert(keys, i)
        if type(v) == 'string' then
            table.insert(values, "'" .. v .. "'")
        else
            table.insert(values, v)
        end
    end
    local str1 = table.concat(keys, ",")
    local str2 = table.concat(values, ",")
    return string.format("INSERT INTO %s (%s) VALUES (%s);", name, str1, str2)
end

function SqlHelper.ReplaceSql(name, tab)
    assert(type(name) == "string")
    assert(type(tab) == "table")
    local keys = { }
    local values = { }
    for i, v in pairs(tab) do
        table.insert(keys, i)
        if type(v) == 'string' then
            table.insert(values, "'" .. v .. "'")
        else
            table.insert(values, v)
        end
    end
    local str1 = table.concat(keys, ",")
    local str2 = table.concat(values, ",")
    return string.format("REPLACE INTO %s (%s) VALUES (%s);", name, str1, str2)
end

function SqlHelper.UpdateSql(name, update, where)
    assert(type(name) == "string")
    assert(type(where) == "table")
    assert(type(update) == "table")
    local updates = { }
    for k, v in pairs(update) do
        if type(v) == "string" then
            table.insert(updates, string.format("%s='%s'", k, v))
        else
            table.insert(updates, string.format("%s=%s", k, tostring(v)))
        end
    end
    local updateString = table.concat(updates, ",")
    local wheres = { }
    for k, v in pairs(where) do
        if type(v) == "string" then
            table.insert(wheres, string.format("%s='%s'", k, v))
        else
            table.insert(wheres, string.format("%s=%s", k, tostring(v)))
        end
    end
    local whereString = table.concat(wheres, ",")
    return string.format("UPDATE %s SET %s %s;", name, updateString, whereString)
end

function SqlHelper.QuerySql(name, keys, where, limit)
    local wheres = { }
    for k, v in pairs(where) do
        if type(v) == "string" then
            table.insert(wheres, string.format("%s='%s'", k, v))
        else
            table.insert(wheres, string.format("%s=%s", k, tostring(v)))
        end
    end
    local fieldString = "*"
    if type(keys) == "table" then
        fieldString = table.concat(keys, ",")
    end
    local whereString = table.concat(wheres, ",")
    if limit == nil then
        return string.format("SELECT % FROM %s WHERE %s;", fieldString, name, whereString)
    end
    return string.format("SELECT %s FROM %s WHERE %s LIMIT %d;", fieldString, name, whereString, limit)
end

return SqlHelper