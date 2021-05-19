MysqlClient = { }

function MysqlClient.InvokeCommand(db, sql)
    if coroutine.running() == nil then
        SoEasy.Error("please use it in the coroutine");
        return nil
    end
    return SoEasy.InvokeMysqlCommand(db, sql)
end

function MysqlClient.Insert(db, tab, value)
    print(db, tab, value)  
    if type(db) ~= 'string' then
        SoEasy.Error("db must be string type");
        return nil
    end

    if type(tab) ~= 'string' then
        SoEasy.Error("tab must be string type");
        return nil
    end
    if type(value) ~= 'table' then
        SoEasy.Error("value must be table type");
        return nil
    end
    local keys = { }
    local values = { }
    for k, v in pairs(value) do
        table.insert(keys, k)
        if type(v) == 'string' then
            table.insert(values, "'" .. v .. "'")
        else
            table.insert(values, v)
        end        
    end
    local sql = string.format("insert into %s(%s)values(%s);", tab, table.concat(keys", "), table.concat(values, ", "))
    print(sql)
    return this.InvokeMysqlCommand(db, sql)
end


return MysqlClient