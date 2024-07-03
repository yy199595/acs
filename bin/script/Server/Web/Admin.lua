local log = require("Log")
local mongo = require("MongoComponent")
local HttpServlet = require("HttpService")
local ADMIN_LIST = "admin_list"

local Admin = HttpServlet()

function Admin:List(request)
    local query = request.query
    local page = tonumber(query.page)
    if page == nil then
        return XCode.CallArgsError
    end
    local filter = { }
    local count = mongo:Count(ADMIN_LIST, filter)
    if count <= 0 then
        return XCode.NotFoundData
    end
    local fields = { "_id", "user_id", "city", "city_name",
                     "permission", "name", "login_ip", "login_time", "create_time" }
    local response = mongo:FindPage(ADMIN_LIST, filter, page, 10, fields)
    if response == nil then
        return XCode.Failure
    end
    return XCode.Ok, { count = count, list = response }
end

function Admin:Menu(request)
    return XCode.Ok, require("Menu")
end

return Admin