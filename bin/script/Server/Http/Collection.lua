local mongo = require("MongoComponent")

local HttpService = require("HttpService")

local Collection = HttpService()

function Collection:FindPage(request)
    local tab = request.query.tab
    local page = request.query.page
    local count = mongo:Count(tab, nil)
    local list = mongo:FindPage(tab, nil, page, nil)
    return XCode.Ok, { count = count, list = list}
end

function Collection:Delete(request)
    local tab = request.data.tab
    table.print(request.data)
    return mongo:Drop(tab)
end

return Collection

