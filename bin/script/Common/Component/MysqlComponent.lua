MysqlComponent = {}

local self
local message
--function MysqlComponent.Awake()
--    self = App.GetComponent("MysqlService")
--    message = App.GetComponent("MessageComponent")
--    return self ~= nil and message ~= nil
--end

function MysqlComponent.Add(tabName, data, flag)
    assert(type(data) == "table")
    assert(type(tabName) == "string")

    local address = self:GetAddress()
    return self:Call(address, "Add",  {
        table = tabName,
        flag = flag or 0,
        data = message:New(tabName, data)
    })
end

function MysqlComponent.Delete(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = self:GetAddress()
    return mysqlService:Call(address, "Delete", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where)
    })
end

function MysqlComponent.QueryOnce(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = self:GetAddress()
    local code, response = self:Call(address, "Query", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where)
    })
    if code ~= XCode.Successful then
        return nil
    end
    return Json.Decode(response.jsons[1])
end

function MysqlComponent.QueryAll(tabName, where, flag)
    assert(type(where) == "table")
    assert(type(tabName) == "string")

    local address = self:GetAddress()
    print(tabName, where, address)
    local code, response = self:Call(address, "Query", {
        table = tabName,
        flag = flag or 0,
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

    local address = self:GetAddress()
    return self:Call(address, "Update", {
        table = tabName,
        flag = flag or 0,
        where_json = Json.Encode(where),
        update_json = Json.Encode(update)
    })
end