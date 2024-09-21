local mongo = require("MongoComponent")

local HttpService = require("HttpService")


local MAX_COUNT = 10
local Collection = HttpService()

function Collection:List(request)
	local collections = mongo:Collections()
    return { code = XCode.Ok, list = collections}
end


function Collection:FindPage(request)

    local tab = request.data.tab
    local page = request.data.page
    local filter = request.data.filter
    local sorter = request.data.sorter
    local count = mongo:Count(tab, filter)
    local list = mongo:FindPage(tab, filter, page, MAX_COUNT, nil, sorter)
    return XCode.Ok, { count = count, list = list}
end

function Collection:Tables()
    local response = { }
    local list = mongo:Databases()
    for _, db in ipairs(list) do
        local item = {
            label = db,
            value = db,
            children = { }
        }
        local collections = mongo:Collections(db)
        for _, tab in ipairs(collections) do
            table.insert(item.children, {
                value = tab,
                label = tab
            })
        end
        table.insert(response, item)
    end
    return XCode.Ok, response
end

function Collection:Delete(request)
    local tab = request.data.tab
    return mongo:Drop(tab)
end

return Collection

