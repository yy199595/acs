
require("TableUtil")
local log = require("Log")
local md5 = require("util.md5")
local json = require("util.json")
local tabName = "user_account"

local app = require("core.app")
local coroutine_start = coroutine.start
local redis = require("RedisComponent")
local mongo = require("MongoComponent")
local HttpService = require("HttpService")

local Account = HttpService()

function Account:Register(account, password)

    local user_data = {
        _id = account,
        login_time = 0,
        account = account,
        password = password,
        create_time = os.time(),
        user_id = app.NewGuid()
    }
    local code = mongo:InsertOnce(tabName, user_data)
    if code ~= XCode.Ok then
        local str = json.encode(user_data)
        log.Error("insert %s code:%s data=%s", tabName, code, str)
        return nil
    end

    log.Debug("user %s register successful data = %s", account, json.encode(user_data))
    return user_data
end

function Account:Verify(request)
    local token = request.data.token
    local response = redis:Run("GET", token)
    if response == nil or response == "" then
        return XCode.Failure
    end

    redis:Run("DEL", token)
    local user_id = tonumber(response)
    return XCode.Ok, { player_id = user_id}
end

function Account:Login(request)

    local message = request.data
    table.print(request)
    assert(type(message.account) == "string", "user account is not string")
    assert(type(message.password) == "string", "user password is not string")

    local user_data = mongo:FindOne(tabName, {
        _id = message.account }, { "password", "user_id" })

    if user_data == nil then
        user_data = self:Register(message.account, message.password)
        if user_data == nil then
            return XCode.InsertMongoDocumentFail
        end
    elseif user_data.password ~= message.password then
        return XCode.AccountPasswordError
    end

    local token = md5.ToString()
    local ip = request.head:Get("x_forwarded_for")
    if ip == nil then
        ip = request.head:Get("X-Real-IP")
    end

    local response = {
        address = "",
        token = token
    }
    local gate_id =  app.Random("GateSystem")
    response.address = app.GetListen(gate_id, "gate")
    if response.address == nil then
        return XCode.AddressAllotFailure
    end
    coroutine_start(function()
        redis:Set(token, user_data.user_id, 300)
        mongo:Update(tabName, {_id = user_data.account}, {
            login_ip = ip,
            login_time = os.time()
        })
    end)
    log.Debug("user %s login successful", message.account)
    return XCode.Ok, response
end

return Account