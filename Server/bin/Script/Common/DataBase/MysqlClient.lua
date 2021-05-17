MysqlClient = { }

function MysqlClient.InvokeCommand(sql)
    if coroutine.running() == nil then
        SoEasy.Error("please use it in the coroutine");
        return nil
    end
    return SoEasy.InvokeMysqlCommand(sql)
end


return MysqlClient