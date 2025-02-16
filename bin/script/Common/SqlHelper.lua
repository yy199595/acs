local SqlHelper = {}
local to_string = tostring
local table_insert = table.insert
local table_concat = table.concat
local string_format = string.format
local json = require("util.json")

function SqlHelper.InsertSql(name, tab)
    assert(type(name) == "string")
    assert(type(tab) == "table")
    local keys = { }
    local values = { }
    for i, v in pairs(tab) do
        table_insert(keys, i)
        if type(v) == 'string' then
            table_insert(values, "'" .. v .. "'")
        elseif type(v) == "table" then
            local str = json.encode(v)
            table_insert(values, "'" .. str .. "'")
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

function SqlHelper.AddFieldSql(name, key, val)
    local sql = "ALTER TABLE %s ADD %s %s NOT NULL DEFAULT %s"
    if type(val) == "number" then
        return string_format(sql, name, key, "int", val)
    elseif type(val) == "string" then
        local data = string_format("'%s'", val)
        return string_format(sql, name, key, "varchar(255)", data)
    elseif type(val) == "table" then
        local default = "{}"
        if next(val) then
            default = json.encode(val)
        end
        local data = string_format("'%s'", default)
        return string_format(sql, name, key, "json", data)
    end
end

function SqlHelper.AddAutoKey(name, key)
   return "ALTER TABLE `%s` AUTO_increment PRIMARY KEY;"
end

function SqlHelper.AddPrimaryKeySql(name, key)
    return string_format("ALTER TABLE %s ADD PRIMARY key (`%s`)", name, key)
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
    if where and next(where) then
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
    if limit == nil or limit == 0 then
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

function SqlHelper.CreateSql(name, tab, index)
    local contents = { }
    for key, val in pairs(tab) do
        if type(val) == "number" then
            if tostring(val):find("%.") == nil then --整数
                if val == 2 then
                    table_insert(contents, string_format("%s SMALLINT NOT NULL DEFAULT %d", key, 0))
                elseif val == 4 then
                    table_insert(contents, string_format("%s INT NOT NULL DEFAULT %d", key, 0))
                else
                    table_insert(contents, string_format("%s BIGINT NOT NULL DEFAULT %d", key, 0))
                end
            elseif val >= 3 and val <= 5 then
                table_insert(contents, string_format("%s FLOAT NOT NULL DEFAULT %d", key, 0))
            else
                table_insert(contents, string_format("%s DOUBLE NOT NULL DEFAULT %d", key, 0))
            end
        elseif type(val) == "string" then
            table_insert(contents, string_format("%s VARCHAR(255) NOT NULL DEFAULT '%s'", key, val))
        elseif type(val) == "table" then
            local str = "DEFAULT NULL";
            if next(val) then
                str = string_format("NOT NULL DEFAULT '%s'", json.encode(val))
            end
            table_insert(contents, string_format("%s JSON %s", key, str))
        end
    end
    if index and next(index) then
        local keys = { }
        for _, v in ipairs(index) do
            table_insert(keys, string_format("`%s`", v))
        end
        local content = table_concat(keys, ",")
        table_insert(contents, string_format("PRIMARY KEY (%s)", content))
    end
    if next(contents) then
        local content = table_concat(contents, ",\n")
        return string_format("CREATE TABLE IF NOT EXISTS %s (\n%s\n);", name, content)
    end
    return string_format("CREATE TABLE IF NOT EXISTS %s ();", name)
end

function SqlHelper.FindPage(name, fields, filter, sorter, page, count)

    local filterString
    if filter and next(filter) then
        local wheres = {}
        for k, v in pairs(filter) do
            if type(v) == "string" then
                table_insert(wheres, string_format("%s='%s'", k, v))
            else
                table_insert(wheres, string_format("%s=%s", k, v))
            end
        end
        filterString = table_insert(wheres, " AND ")
    end

    -- 构建选择的字段列表
    local fieldString = "*"
    if fields and next(fields) then
        fieldString = table_concat(fields, ',')
    end

    -- 构建排序条件
    local orderByString
    if sorter and next(sorter) then
        local orderClauses = {}
        for k, v in pairs(sorter) do
            local direction = (v == 1) and "ASC" or (v == -1) and "DESC" or "ASC"
            table_insert(orderClauses, string_format("%s %s", k, direction))
        end
        orderByString = "ORDER BY " .. table_concat(orderClauses, ", ")
    end

    -- 构建分页条件
    local offset = (page - 1) * count
    local limitString = string_format("LIMIT %d, %d", offset, count)

    -- 完整的查询语句
    local sql = string_format("SELECT %s FROM %s", fieldString, name)

    if filterString then
        sql = sql .. " WHERE " .. filterString
    end
    if orderByString then
        sql = sql .. " " .. orderByString
    end
    sql = sql .. " " .. limitString .. ";"
    return sql
end

function SqlHelper.Count(tab, filter)
    local whereString
    if filter and next(filter) then
        local wheres = { }
        for k, v in pairs(filter) do
            if type(v) == "string" then
                table_insert(wheres, string_format("%s='%s'", k, v))
            else
                table_insert(wheres, string_format("%s=%s", k, to_string(v)))
            end
        end
        whereString = table_concat(wheres, ",")
    end
    if whereString == nil then
        return string_format("SELECT COUNT(*) FROM %s", tab)
    end
    return string_format("SELECT COUNT(*) FROM %s WHERE %s", tab, whereString)
end

return SqlHelper