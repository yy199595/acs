
MongoComponent = {}
local mongoService = App.GetComponent("MongoService")

function MongoComponent.InsertOnce(tab, data, flag)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    local address = mongoService:GetHost()
    return mongoService:Call(address, "Insert", {
        tab = tab,
        json = data,
        flag = flag or 0
    })
end

function MongoComponent.Delete(tab, data, limit, flag)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    assert(type(data) == "string")
    local address = mongoService:GetHost()
    return mongoService:Call(address, "Delete", {
        tab = tab,
        json = data,
        limit = limit or 1,
        flag = flag or 0
    })
end

function MongoComponent.QueryOnce(tab, data)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    assert(type(data) == "string")
    local address = mongoService:GetHost()
    local code, response = mongoService:Call(address, "Query", {
        tab = tab,
        json = data,
        limit = 1
    })
    if code == XCode.Successful then
        if response.jsons and #response.jsons > 0 then
            return Json.Decode(response.jsons[1])
        end
    end
    return nil
end

function MongoComponent.Query(tab, data, limit)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    local responses = {}
    assert(type(data) == "string")
    local address = mongoService:GetHost()
    local code, response = mongoService:Call(address, "Query", {
        tab = tab,
        json = data,
        limit = limit or 0
    })

    if code == XCode.Successful and response.jsons and #response.jsons > 0 then
        for _, json in ipairs(response.jsons) do
            table.insert(responses, Json.Decode(json))
        end
    end
    return responses
end

function MongoComponent.SetIndex(tab, name)
    assert(type(tab) == "string")
    assert(type(name) == "string")
    local address = mongoService:GetHost()
    return mongoService:Call(address, "SetIndex", {
        tab = tab,
        name = name
    })
end

function MongoComponent.Update(tab, select, update, tag, flag)
    if type(select) == "table" then
        select = Json.Encode(select)
    end
    if type(update) == "table" then
        update = Json.Encode(update)
    end
    assert(type(select) == "string")
    assert(type(update) == "string")
    local address = mongoService:GetHost()
    return mongoService:Call(address, "Update", {
        tab = tab,
        select = select,
        update = update,
        tag = tag or "$set",
        flag = flag or 0
    })
end

function MongoComponent.Push(tab, select, update)
    return MongoComponent.Update(tab, select, update, "$push")
end

function MongoComponent.AddCounter(key)
    local tab = "custom_counter";
    return MongoComponent.Update(tab, {
        _id = key
    }, {value = 1}, "$inc")
end

return MongoComponent