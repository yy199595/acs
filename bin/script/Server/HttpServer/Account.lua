
local md5 = require("Md5")
local tabName = "user.account_info"
local log_warn = require("Log").Warning
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local AccountService = Class("HttpService")

function AccountService:Awake()

    self.redis = redis
    self.app:AddWatch("Gate")
    self.app:AddWatch("MongoDB")
end

function AccountService:Complete()
    local code = mongo:Drop(tabName)
    self.session = self.app:Allot("MongoDB")
end

function AccountService:NewToken(user_id)
    local str = tostring(os.time())
    return md5.ToString(str .. tostring(user_id))
end

function AccountService:Register(request)

    local message = request.message
    assert(message.account, "register account is nil")
    assert(message.password, "register password is nil")

    local account_info = mongo:FindOne(tabName, { _id = message.account}, { "user_id"})
    if account_info ~= nil then
        log_warn(message.account, " already exists")
        return XCode.AccountAlreadyExists
    end

    local user_id = self.redis:AddCounter("user_id", 10000)
    return mongo:InsertOnce(tabName, {
        _id = message.account,
        user_id = user_id,
        login_time = 0,
        account = message.account,
        password = message.password,
        create_time = os.time(),
        last_login_ip = message.from,
        auth_token = ""
    })
end

function AccountService:Login(request)

    local message = request.message
    assert(type(message.account) == "string", "user account is not string")
    assert(type(message.password) == "string", "user password is not string")

    local response = mongo:FindOne(tabName, {
        _id = message.account }, { "password", "user_id" })

    log_warn(response.password, message.password)
    if response == nil or response.password ~= message.password then
        print("账号密码错误")
        return XCode.AccountPasswordError
    end

    local session = self.app:Allot("Gate")
    local token = self:NewToken(response.user_id)
    local code = self.app:Call(session, "Gate.Enter", {
        token = token,
        user_id = response.user_id
    })

    if code ~= XCode.Successful then
        return XCode.AddressAllotFailure
    end
    local gate = self.app:GetListen(session, "gate")
    return XCode.Successful, { token = token, address = gate }
end

return AccountService