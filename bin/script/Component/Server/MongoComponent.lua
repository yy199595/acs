
local MongoComponent = {}
local this = MongoComponent
function MongoComponent.InsertOnce(tab, data, flag)
    if type(data) == "table" then
        data = rapidjson.encode(data)
    end
    assert(type(flag) == "number")
    local address = Service.AllotServer("MongoDB")
    return Service.Call(address, "MongoDB.Insert", {
        tab = tab,
        json = data,
        flag = flag
    })
end

function MongoComponent.Delete(tab, data, limit, flag)
    if type(data) == "table" then
        data = rapidjson.encode(data)
    end
    assert(type(data) == "string")
    assert(type(flag) == "number")
    local address = Service.AllotServer("MongoDB")
    return Service.Call(address, "MongoDB.Delete", {
        tab = tab,
        json = data,
        limit = limit or 1,
        flag = flag
    })
end

function MongoComponent.ClearTable(tab)
    assert(type(tab) == "string")
    return this.Delete(tab, {}, 0, 0) == XCode.Successful
end

function MongoComponent.QueryOnce(tab, data)
    if type(data) == "table" then
        data = rapidjson.encode(data)
    end
    assert(type(data) == "string")
    local address = Service.AllotServer("MongoDB")
    local code, response = Service.Call(address, "MongoDB.Query", {
        tab = tab,
        json = data,
        limit = 1
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    if #response.jsons > 0 then
        return rapidjson.decode(response.jsons[1])
    end
    return nil
end

function MongoComponent.Query(tab, where, limit)
    if type(where) == "table" then
        where = rapidjson.encode(where)
    end
    print("query json ", where)
    assert(type(where) == "string")
    local address = Service.AllotServer("MongoDB")
    local code, response = Service.Call(address, "MongoDB.Query", {
        tab = tab,
        json = data,
        limit = limit or 0
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    local responses = {}
    if #response.jsons > 0 then
        for _, json in ipairs(response.jsons) do
            table.insert(responses, rapidjson.decode(json))
        end
    end
    return responses
end
-- 同时查询多个 _id,参数是匹配的_id列表
function MongoComponent.QueryWheres(tab, wheres)
    assert(type(tab) == "string")
    assert(type(wheres) == "table" and #list > 0)

    local request = {
        _id = {
            ["$in"] = wheres
        }
    }
    local address = Service.AllotServer("MongoDB")
    local code, response = Service.Call(address, "MongoDB.Query", {
        tab = tab,
        limit = #list,
        json = rapidjson.encode(request)
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    local responses = {}
    if #response.jsons > 0 then
        for _, json in ipairs(response.jsons) do
            table.insert(responses, rapidjson.decode(json))
        end
    end
    return responses
end

function MongoComponent.SetIndex(tab, name)
    assert(type(tab) == "string")
    assert(type(name) == "string")
    local address = Service.AllotServer("MongoDB")
    return Service.Call(address, "MongoDB.SetIndex", {
        tab = tab,
        name = name
    })
end

function MongoComponent.Update(tab, select, update, flag, tag)
    if type(select) == "table" then
        select = rapidjson.encode(select)
    end

    if type(update) == "table" then
        update = rapidjson.encode(update)
    end
    assert(type(flag) == "number")
    assert(type(select) == "string")
    assert(type(update) == "string")
    local address = Service.AllotServer("MongoDB")
    return Service.Call(address, "MongoDB.Update", {
        tab = tab,
        select = select,
        update = update,
        tag = tag or "$set",
        flag = flag
    })
end

function MongoComponent.Push(tab, select, update, flag)
    return this.Update(tab, select, update, flag,"$push")
end

return MongoComponent