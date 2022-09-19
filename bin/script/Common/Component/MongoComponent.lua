
MongoComponent = {}
MongoComponent.Counters = { }

local this = MongoComponent
local self = App.GetComponent("MongoService")
function MongoComponent.InsertOnce(tab, data, flag)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    local address = self:GetHost()
    return self:Call(address, "Insert", {
        tab = tab,
        json = data,
        flag = flag or 0
    })
end

function MongoComponent.Delete(tab, data, limit, flag)
    if type(data) == "table" then
        data = Json.Encode(data)
    end
    local address = self:GetHost()
    assert(type(data) == "string")
    return self:Call(address, "Delete", {
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
    local address = self:GetHost()
    assert(type(data) == "string")
    local code, response = self:Call(address, "Query", {
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
    local address = self:GetHost()
    local code, response = self:Call(address, "Query", {
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

    local address = self:GetHost()
    local requset = {
        _id = {
            ["$in"] = querys
        }
    }
    local code, response = self:Call(address, "Query", {
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
    local address = self:GetHost()
    return self:Call(address, "SetIndex", {
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

    local address = self:GetHost()
    return self:Call(address, "Update", {
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

function MongoComponent.AddCounter(key, value)

    local address = self:GetHost()
    local code, response = self:Call(address, "AddCounter", {
        key = key,
        value = value
    })
    return code == XCode.Successful and response.value or 0
end

function MongoComponent.GetCount(tab, query)
    local requset = { }
    requset.count = tab
    if type(query) == "table" then
        requset.query = query
    end
    local response = this.RunCommand(requset)
    return response.n
end

function MongoComponent.RunCommand(parameter)
    local address = self:GetHost()
    local json = Json.Encode(parameter)
    local code, response = self:Call(address, "RunCommand", {
        tab = tab,
        cmd = cmd,
        json = json
    })
    if code ~= XCode.Successful or response == nil then
        return nil
    end
    if #response.jsons == 1 then
        return Json.Decode(response.jsons[1])
    else
        local res = {}
        for _, str in ipairs(response.jsons) do
            local result = json.Decode(str)
            table.insert(res, result)
        end
        return res
    end
end

function MongoComponent.RumTableCommand(tab, cmd, parameter)

    assert(type(tab) == "string")
    assert(type(cmd) == "string")
    assert(type(parameter) == "table")
    parameter[cmd] = tab
    return this.RunCommand(parameter)
end

return MongoComponent