local log = require("Log")
local mongo = require("MongoComponent")
local HttpServlet = require("HttpService")
local ADMIN_LIST = "admin_list"

local Admin = HttpServlet()

function Admin:Menu(request)
    package.loaded["Menu"] = nil
    return XCode.Ok, require("Menu")
end

return Admin