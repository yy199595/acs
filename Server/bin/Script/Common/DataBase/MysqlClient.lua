MysqlClient = { }

function MysqlClient.InvokeCommand(db, sql)
    if coroutine.running() == nil then
        SoEasy.Error("please use it in the coroutine");
        return nil
    end
    return SoEasy.InvokeMysqlCommand(db, sql)
end


return MysqlClient