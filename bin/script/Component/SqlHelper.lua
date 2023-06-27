local SqlHelper = {}
local to_string = tostring
local table_insert = table.insert
local table_concat = table.concat
local string_format = string.format
local json = rapidjson

function SqlHelper.InsertSql(name, tab)
    assert(type(name) == "string")
    assert(type(tab) == "table")
    local keys = { }
    local values = { }
    for i, v in pairs(tab) do
        table_insert(keys, i)
        if type(v) == 'string' then
            table_insert(values, "'" .. v .. "'")
        else
            table_insert(values, v)
        end
    end
    local str1 = table_concat(keys, ",")
    local str2 = table_concat(values, ",")
    return string_format("INSERT INTO %s (%s) VALUES (%s);", name, str1, str2)
end

function SqlHelper.ReplaceSql(name, tab)
    assert(type(name) == "string")
    assert(type(tab) == "table")
    local keys = { }
    local values = { }
    for i, v in pairs(tab) do
        table_insert(keys, i)
        if type(v) == 'string' then
            table_insert(values, "'" .. v .. "'")
        else
            table_insert(values, v)
        end
    end
    local str1 = table_concat(keys, ",")
    local str2 = table_concat(values, ",")
    return string_format("REPLACE INTO %s (%s) VALUES (%s);", name, str1, str2)
end

function SqlHelper.UpdateSql(name, update, where)
    assert(type(name) == "string")
    assert(type(where) == "table")
    assert(type(update) == "table")
    local updates = { }
    for k, v in pairs(update) do
        if type(v) == "string" then
            table_insert(updates, string_format("%s='%s'", k, v))
        else
            table_insert(updates, string_format("%s=%s", k, to_string(v)))
        end
    end
    local updateString = table_concat(updates, ",")
    local wheres = { }
    for k, v in pairs(where) do
        if type(v) == "string" then
            table_insert(wheres, string_format("%s='%s'", k, v))
        else
            table_insert(wheres, string_format("%s=%s", k, to_string(v)))
        end
    end
    local whereString = table_concat(wheres, ",")
    return string_format("UPDATE %s SET %s %s;", name, updateString, whereString)
end

function SqlHelper.QuerySql(name, fields, where, limit)
    local whereString
    if type(where) == "table" then
        local wheres = { }
        for k, v in pairs(where) do
            if type(v) == "string" then
                table_insert(wheres, string_format("%s='%s'", k, v))
            else
                table_insert(wheres, string_format("%s=%s", k, to_string(v)))
            end
        end
        whereString = table_concat(wheres, ",")
    end
    local fieldString = "*"
    if type(fields) == "table" then
        fieldString = table_concat(fields, ",")
    end
    if limit == nil then
        if whereString == nil then
            return string_format("SELECT %s FROM %s;", fieldString, name)
        end
        return string_format("SELECT %s FROM %s WHERE %s;", fieldString, name, whereString)
    end
    if whereString == nil then
        return nil
    end
    return string_format("SELECT %s FROM %s WHERE %s LIMIT %d;", fieldString, name, whereString, limit)
end

function SqlHelper.DeleteSql(name, where)
    local wheres = { }
    for k, v in pairs(where) do
        if type(v) == "string" then
            table_insert(wheres, string_format("%s='%s'", k, v))
        else
            table_insert(wheres, string_format("%s=%s", k, to_string(v)))
        end
    end
    local whereString = table_concat(wheres, ",")
    return string_format("DELETE FROM %s WHERE %s;", name, whereString)
end

function SqlHelper.CreateSql(name, tab)
    local contents = { }
    for key, val in pairs(tab) do
        if type(val) == "number" then
            table_insert(contents, string_format("%s int NOT NULL DEFAULT %d", key, val))
        elseif type(val) == "string" then
            table_insert(contents, string_format("%s varchar(255) NOT NULL DEFAULT '%s'", key, val))
        elseif type(val) == "table" then
            table_insert(contents, string_format("%s json NOT NULL DEFAULT %d", key, json.encode(val)))
        end
    end
    local content = table_concat(contents, ",\n")
    return string_format("CREATE TABLE %s (\n%s\n);", name, content)
end

return SqlHelper