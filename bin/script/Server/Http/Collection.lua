

local HttpService = require("HttpService")
local mongo = require("MongoProxyComponent")

local MAX_COUNT = 10
local typeof = type

local app = require("App")
local table_insert = table.insert
local Collection = HttpService()

function Collection:List(request)
    return XCode.Ok
end

local INT32_MAX_VALUE = 2^31 - 1

function Collection:GenFilter(filter)
    local query
    if filter.key and filter.value then

        local value = filter.value
        if filter.type == "int32" or filter.type == "int64" then
            value = tonumber(filter.value)
        end

        if filter.opt == "" then
            query = {
                [filter.key] = value
            }
        else
            query = {
                [filter.key] = {
                    [filter.opt] = value
                }
            }
        end
    end
    return query
end

function Collection:FindPage(request)

    local tab = request.data.tab
    local page = request.data.page
    local filter = request.data.filter
    local sorter = request.data.sorter

    local query = self:GenFilter(filter)
    local count = mongo:Count(tab, query)
    local list = mongo:FindPage(tab, query, page, MAX_COUNT, nil, sorter)
    if list == nil or #list == 0 then
        return XCode.Ok, { count = count, list = list}
    end

    local fieldTypes = { }
    for _, document in ipairs(list) do
        for k, value in pairs(document) do
            if fieldTypes[k] == nil then
                local tt = typeof(value)
                if tt == "number" then
                    if math.tointeger(value) == nil then
                        tt = "float"
                    elseif value >= INT32_MAX_VALUE then
                        tt = "int64"
                    else
                        tt = "int32"
                    end
                elseif tt == "table" then
                    tt = "object"
                    if value.n and #value > 0 then
                        tt = "array"
                    end
                end
                fieldTypes[k] = tt
            end
            if type(value) == "number" and value >= INT32_MAX_VALUE then
                document[k] = tostring(value)
            end
        end
    end
    return XCode.Ok, { count = count, list = list, types = fieldTypes }
end

function Collection:FindAll(request)
    local tab = request.data.tab
    local filter = request.data.filter
    local query = self:GenFilter(filter)
    if request.data.cursor then
        local index = tonumber(request.data.cursor)
        local documents, cursor = mongo:GetMore(tab, index, 500)
        return XCode.Ok, { list = documents, cursor = tostring(cursor)}
    end
    local documents, cursor = mongo:Find(tab, query, nil, 500)
    return XCode.Ok, { list = documents, cursor = tostring(cursor)}
end

function Collection:Count(request)
    local filter = request.data.filter

    local tab = request.data.tab
    local query = self:GenFilter(filter)
    local count = mongo:Count(tab, query)
    return XCode.Ok, { count = count }
end

function Collection:Tables()
    local response = { }
    local list = mongo:ListDatabases()
    for _, db in ipairs(list) do
        local item = {
            label = db.name,
            value = db.name,
            children = { }
        }
        local collections = mongo:ListCollections(db.name)
        for _, tab in ipairs(collections) do
            table_insert(item.children, {
                value = tab.name,
                label = tab.name
            })
        end
        table_insert(response, item)
    end
    return XCode.Ok, { list = response }
end

function Collection:Delete(request)
    local tab = request.data.tab
    local filter = request.data.filter
    return mongo:DeleteOne(tab, filter)
end

return Collection

