
local MongoComponent = {}
MongoComponent.Counters = { }

local this = MongoComponent
function MongoComponent.InsertOnce(tab, data, flag)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    local list = Service.GetServerList("MongoService")
    table.print(list)
    local address = Service.AllotServer("MongoService")
    return Service.Call(address, "MongoService.Insert", {
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
    local address = Service.AllotServer("MongoService")
    return Service.Call(address, "MongoService.Delete", {
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
    local address = Service.AllotServer("MongoService")
    local code, response = Service.Call(address, "MongoService.Query", {
        tab = tab,
        json = data,
        limit = 1
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    if #response.jsons > 0 then
        return Json.Decode(response.jsons[1])
    end
    return nil
end

function MongoComponent.Query(tab, data, limit)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    print("query json ", data)
    assert(type(data) == "string")
    local address = Service.AllotServer("MongoService")
    local code, response = Service.Call(address, "MongoService.Query", {
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
            table.insert(responses, Json.Decode(json))
        end
    end
    return responses
end

function MongoComponent.QueryDatas(tab, querys)
    assert(type(tab) == "string")
    assert(type(querys) == "table" and #querys > 0)

    local requset = {
        _id = {
            ["$in"] = querys
        }
    }
    local address = Service.AllotServer("MongoService")
    local code, response = Service.Call(address, "MongoService.Query", {
        tab = tab,
        limit = #querys,
        json = Json.Encode(requset)
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    local responses = {}
    if #response.jsons > 0 then
        for _, json in ipairs(response.jsons) do
            table.insert(responses, Json.Decode(json))
        end
    end
    return responses
end

function MongoComponent.SetIndex(tab, name)
    assert(type(tab) == "string")
    assert(type(name) == "string")
    local address = Service.AllotServer("MongoService")
    return Service.Call(address, "MongoService.SetIndex", {
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
    local address = Service.AllotServer("MongoService")
    return Service.Call(address, "MongoService.Update", {
        tab = tab,
        select = select,
        update = update,
        tag = tag or "$set",
        flag = flag or 0
    })
end

function MongoComponent.Push(tab, select, update)
    return this.Update(tab, select, update, "$push")
end

return MongoComponent