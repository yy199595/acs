
MongoComponent = {}
local mongoService
local messageComponent
function MongoComponent.Awake()
    mongoService = App.GetComponent("MongoService")
    messageComponent = App.GetComponent("MessageComponent")
end

function MongoComponent.InsertOnce(tab, data)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    local request = messageComponent:New("s2s.Mongo.Insert", {
        tab = tab,
        json = data
    })
    local address = mongoService:GetAddress()
    return mongoService:Call(address, "Insert", request)
end

function MongoComponent.Delete(tab, data, limit)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    assert(type(data) == "string")
    local request = messageComponent:New("s2s.Mongo.Delete", {
        tab = tab,
        json = data,
        limit = limit or 1
    })
    local address = mongoService:GetAddress()
    return mongoService:Call(address, "Delete", request)
end

function MongoComponent.QueryOnce(tab, data)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    assert(type(data) == "string")
    local request = messageComponent:New("s2s.Mongo.Query.Request", {
        tab = tab,
        json = data,
        limit = 1
    })
    local address = mongoService:GetAddress()
    local response = mongoService:Call(address, "Query", request)

    if response.jsons and #response.jsons > 0 then
        return Json.Decode(response.jsons[1])
    end
    return nil
end

function MongoComponent.Query(tab, data, limit)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    assert(type(data) == "string")
    local request = messageComponent:New("s2s.Mongo.Query.Request", {
        tab = tab,
        json = data,
        limit = limit or 0
    })
    local responses = {}
    local address = mongoService:GetAddress()
    local response = mongoService:Call(address, "Query", request)

    if response.jsons and #response.jsons > 0 then
        for _, json in ipairs(response.jsons) do
            table.insert(responses, Json.Decode(json))
        end
    end
    return responses
end

function MongoComponent.Update(tab, select, update, tag)
    if type(select) == "table" then
        select = Json.Encode(select)
    end
    if type(update) == "table" then
        update = Json.Encode(update)
    end
    assert(type(select) == "string")
    assert(type(update) == "string")
    local request = messageComponent:New("s2s.Mongo.Query.Request", {
        tab = tab,
        select = select,
        update = update,
        tag = tag or "$set"
    })
    local address = mongoService:GetAddress()
    return mongoService:Call(address, "Update", request)
end

return MongoComponent