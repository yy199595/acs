MysqlClient = { }
local this = MysqlClient
function MysqlClient.InvokeCommand(sql)
    if coroutine.running() == nil then
        SoEasy.Error("please use it in the coroutine");
        return nil
    end
    return SoEasy.InvokeMysqlCommand(sql)
end

function MysqlClient.Insert(tab, value)
    if not this.CheckArgv(tab, value) then
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
    local k1 = table.concat(keys, ",")
    local v1 = table.concat(values, ",")
    local sql = string.format("insert into %s(%s)values(%s)", tab, k1, v1)
    local json = this.InvokeCommand(sql)
    return json.code == 0
end

function MysqlClient.CheckArgv(tab, value)
    if type(tab) ~= 'string' then
        SoEasy.Error("tab must be string type");
        return false
    end
    if type(value) ~= 'table' then
        SoEasy.Error("value must be table type");
        return false
    end
    return true
end


return MysqlClient