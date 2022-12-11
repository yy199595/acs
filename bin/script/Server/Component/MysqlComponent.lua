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
    assert(type(flag) == "number")
    assert(type(tabName) == "string")
    local address = Service.AllotLocation()
    return Service.Call(address, "MysqlService.Add",  {
        table = tabName,
        flag = flag,
        data = Proto.New(tabName, data)
    })
end

function MysqlComponent.Delete(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(flag) == "number")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    return Service.Call(address, "MysqlService.Delete", {
        table = tabName,
        flag = flag,
        where_json = rapidjson.encode(where)
    })
end

function MysqlComponent.QueryOnce(tabName, where)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    local code, response = Service.Call(address, "MysqlService.Query", {
        limit = 1,
        table = tabName,
        where_json = rapidjson.encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    return rapidjson.decode(response.jsons[1])
end

function MysqlComponent.QueryAll(tabName, where, limit)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    print(tabName, where, address)
    local address = Service.AllotLocation("MysqlService")
    local code, response = Service.Call(address, "MysqlService.Query", {
        table = tabName,
        limit = limit or 0,
        where_json = rapidjson.encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.jsons) do
        table.insert(res, rapidjson.decode(json))
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
        where_json = rapidjson.encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.jsons) do
        table.insert(res, rapidjson.encode(json))
    end
    return res
end

function MysqlComponent.Update(tabName, where, update, flag)
    assert(type(where) == "table")
    assert(type(flag) == "number")
    assert(type(update) == "table")
    assert(type(tabName) == "string")

    local address = Service.AllotLocation("MysqlService")
    return Service.Call(address, "MysqlService.Update", {
        flag = flag,
        table = tabName,
        where_json = rapidjson.encode(where),
        update_json = rapidjson.encode(update)
    })
end
return MysqlComponent