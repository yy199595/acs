MysqlComponent = {}
local mysqlService
local messageComponent
function MysqlComponent.Awake()
    mysqlService = App.GetComponent("MysqlService")
    messageComponent = App.GetComponent("MessageComponent")
    return mysqlService and messageComponent
end

function MysqlComponent.Start()
    Log.Warning("mysql component start")
    return true
end

function MysqlComponent.Add(tabName, data, flag)
    assert(type(data) == "table")
    assert(type(tabName) == "string")
    local address = mysqlService:GetAddress()
    return mysqlService:Call(address, "Add",  {
        table = tabName,
        flag = flag or 0,
        data = messageComponent:New(tabName, data)
    })
end

function MysqlComponent.Delete(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")
    local address = mysqlService:GetAddress()
    return mysqlService:Call(address, "Delete", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where)
    })
end

function MysqlComponent.QueryOnce(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")
    local address = mysqlService:GetAddress()
    local code, response = mysqlService:Call(address, "Query", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    return Json.Decode(response.json_array[1])
end

function MysqlComponent.QueryOnce(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")
    local address = mysqlService:GetAddress()
    local code, response = mysqlService:Call(address, "Query", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.json_array) do
        table.insert(res, Json.Decode(json))
    end
    return res
end

function MysqlComponent.Update(tabName, where, update, flag)
    assert(type(where) == "table")
    assert(type(update) == "table")
    assert(type(tabName) == "string")
    local address = mysqlService:GetAddress()
    return mysqlService:Call(address, "Update", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where),
        update_json = Json.Encode(update)
    })
end