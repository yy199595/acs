local MysqlComponent = {}

local json = rapidjson
require("XCode")
local proto = require("Proto")
local rpcService = require("Service")
local mysqlService = rpcService.New("MysqlDB")
function MysqlComponent.Create(tabName, keys, data)
    local request = proto.New(tabName, data)
    return mysqlService:Call(nil, "Create", {
        keys = keys,
        data = request
    })
end

function MysqlComponent.Add(tabName, data, flag)
    assert(type(data) == "table")
    assert(type(flag) == "number")
    assert(type(tabName) == "string")
    return mysqlService:Call(nil, "Add", {
        table = tabName,
        flag = flag,
        data = proto.New(tabName, data)
    })
end

function MysqlComponent.Delete(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(flag) == "number")
    assert(type(tabName) == "string")

    return mysqlService:Call(nil, "MysqlDB.Delete", {
        table = tabName,
        flag = flag,
        where_json = json.encode(where)
    })
end

function MysqlComponent.QueryOnce(tabName, where)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local code, response = mysqlService:Call(address, "Query", {
        limit = 1,
        table = tabName,
        where_json = json.encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    return json.decode(response.jsons[1])
end

function MysqlComponent.QueryAll(tabName, where, limit)
    assert(type(where) == "table")
    assert(type(tabName) == "string")
    local code, response = mysqlService:Call(nil, "Query", {
        table = tabName,
        limit = limit or 0,
        where_json = json.encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.jsons) do
        table.insert(res, json.decode(json))
    end
    return res
end

function MysqlComponent.QueryFields(tabName, fields, where, limit)
    assert(type(fields) == "table")
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local code, response = mysqlService:Call(nil, "Query", {
        table = tabName,
        limit = limit or 0,
        fields = fields,
        where_json = json.encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    local res = { }
    for _, json in ipairs(response.jsons) do
        table.insert(res, json.encode(json))
    end
    return res
end

function MysqlComponent.Update(tabName, where, update, flag)
    assert(type(where) == "table")
    assert(type(flag) == "number")
    assert(type(update) == "table")
    assert(type(tabName) == "string")

    return mysqlService:Call(nil, "MysqlDB.Update", {
        flag = flag,
        table = tabName,
        where_json = json.encode(where),
        update_json = json.encode(update)
    })
end

return MysqlComponent