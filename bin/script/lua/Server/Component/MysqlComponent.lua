local MysqlComponent = {}

function MysqlComponent.Create(tabName, keys, data)
    local request = Proto.New(tabName, data)
    local address = Service.AllotLocation("MysqlService")
    return Service.Call(address, "MysqlService.Create", {
        keys = keys,
        data = request
    })
end

function MysqlComponent.Add(tabName, data, flag)
    assert(type(data) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation()
    return Service.Call(address, "MysqlService.Add",  {
        table = tabName,
        flag = flag or 0,
        data = Proto.New(tabName, data)
    })
end

function MysqlComponent.Delete(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    return Service.Call(address, "MysqlService.Delete", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where)
    })
end

function MysqlComponent.QueryOnce(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    local code, response = Service.Call(address, "MysqlService.Query", {
        limit = 1,
        table = tabName,
        where_json = Json.Encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    return Json.Decode(response.jsons[1])
end

function MysqlComponent.QueryAll(tabName, where, limit)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    print(tabName, where, address)
    local address = Service.AllotLocation("MysqlService")
    local code, response = Service.Call(address, "MysqlService.Query", {
        table = tabName,
        limit = limit or 0,
        where_json = Json.Encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.jsons) do
        table.insert(res, Json.Decode(json))
    end
    return res
end

function MysqlComponent.QueryFields(tabName, fields, where, limit)
    assert(type(fields) == "table")
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    local code, response = Service.Call(address, "MysqlService.Query", {
        table = tabName,
        limit = limit or 0,
        fields = fields,
        where_json = Json.Encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.jsons) do
        table.insert(res, Json.Decode(json))
    end
    return res
end

function MysqlComponent.Update(tabName, where, update, flag)
    assert(type(where) == "table")
    assert(type(update) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    return Service.Call(address, "MysqlService.Update", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where),
        update_json = Json.Encode(update)
    })
end