
require("TableUtil")
local log = require("Log")
local jwd = require("Jwt")
local ADMIN_LIST = "admin_list"
local sqlite = require("SqliteComponent")
local HttpServlet = require("HttpService")

local Admin = HttpServlet()

function Admin:OnAwake()

    --sqlite:Drop(ADMIN_LIST)

    --table.print(sqlite:Query("sqlite_master"))

    if not sqlite:IsExist(ADMIN_LIST) then
        sqlite:Create(ADMIN_LIST, {
            name = "",
            user_id = 4,
            account = "",
            password = "",
            login_time = 8,
            permission = 4,
            create_time = 8
        }, { "user_id", "account" })

        sqlite:SetIndex(ADMIN_LIST, "user_id", true)
        sqlite:SetIndex(ADMIN_LIST, "account", true)
    end

    if sqlite:FindOne(ADMIN_LIST, { account = "root" }) == nil then
        sqlite:Insert(ADMIN_LIST, {
            user_id = 1000,
            account = "root",
            password = "root123",
            login_time = 0,
            create_time = os.time(),
            permission = 100,
            name = "超级管理员"
        })
    end
end

function Admin:OnComplete()

end

function Admin:Menu(request)
    package.loaded["Http.Menu"] = nil
    return XCode.Ok, require("Http.Menu")
end

function Admin:Login(request)

    local password = request.query.passwd
    local filter = { account = request.query.user }
    local userInfo = sqlite:FindOne(ADMIN_LIST, filter)
    if userInfo == nil then
        return XCode.AccountNotExists
    end
    if userInfo.password ~= password then
        return XCode.AccountPasswordError
    end
    userInfo.login_ip = request.head:Get("X-Forwarded-For")
    if userInfo.login_ip == nil then
        userInfo.login_ip = request.head:Get("X-Real-IP")
    end
    local token = jwd.Create({
        t = 0,
        u = userInfo.use_id,
        p = userInfo.permission
    })
    userInfo.login_time = os.time()

    sqlite:UpdateOne(ADMIN_LIST, filter, {
        login_time = userInfo.login_time
    })

    return XCode.Ok, {
        token = token,
        exp_time = 0,
        name = userInfo.name,
        user_id = userInfo.use_id,
        login_ip = userInfo.login_ip,
        login_time = userInfo.login_time,
        permission = userInfo.permission,
        create_time = userInfo.create_time
    }
end

function Admin:List(request)

end

function Admin:Register(request)

end

return Admin