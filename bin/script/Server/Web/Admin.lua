local log = require("Log")
local jwd = require("Jwt")

local ADMIN_LIST = "admin_list"
local HttpServlet = require("HttpService")
local _, mongo = pcall(require, "MongoComponent")
local Admin = HttpServlet()

function Admin:OnAwake()
    self.accounts = { }
    table.insert(self.accounts, {
        use_id = 1,
        account = "root",
        password = "123456",
        permission = 100,
        name = "超级管理员"
    })
end

function Admin:Menu(request)
    package.loaded["Http.Menu"] = nil
    return XCode.Ok, require("Http.Menu")
end

function Admin:GetUser(account)
    for _, info in ipairs(self.accounts) do
        if info.account == account then
            return info
        end
    end
    if mongo ~= nil then
        local response = mongo:FindOne(ADMIN_LIST, { account = account})
        if response == nil then
            return nil
        end
        table.insert(self.accounts, response)
        return response
    end
end

function Admin:Login(request)

    local account = request.query.user
    local password = request.query.passwd
    local userInfo = self:GetUser(account)
    log.Debug("{} {}", request.query, userInfo)
    if userInfo == nil then
        return XCode.AccountNotExists
    end
    if userInfo.password ~= password then
        return XCode.AccountPasswordError
    end
    userInfo.login_ip  = request.head:Get("X-Forwarded-For")
    if userInfo.login_ip  == nil then
        userInfo.login_ip  = request.head:Get("X-Real-IP")
    end
    local token = jwd.Create({
        t = 0,
        u = userInfo.use_id,
        p = userInfo.permission
    })
    userInfo.login_time = os.time()

    local info = {
        login_ip = ip,
        name = userInfo.name,
        user_id = userInfo.use_id,
        login_time = os.time(),
        permission = userInfo.permission,
        create_time = userInfo.create_time
    }
    if mongo ~= nil then
        mongo:Update(ADMIN_LIST, { use_id = userInfo.userInfo }, info)
    end

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