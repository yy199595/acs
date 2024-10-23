
require("TableUtil")
local log = require("Log")
local jwt = require("Jwt")
local md5 = require("util.md5")
local json = require("util.json")
local tabName = "user_account"

local app = require("core.app")
local coroutine_start = coroutine.start
local mongo = require("MongoComponent")
local HttpService = require("HttpService")

local EXP_TIME = 5 * 60

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

function Account:Login(request)

    local message = request.data
    assert(type(message.account) == "string", "user account is not string")
    assert(type(message.password) == "string", "user password is not string")

    local user_data = mongo:FindOne(tabName, {
        _id = message.account }, { "password", "user_id" })

    table.print(user_data)
    if user_data == nil then
        user_data = self:Register(message.account, message.password)
        if user_data == nil then
            return XCode.InsertMongoDocumentFail
        end
    elseif user_data.password ~= message.password then
        return XCode.AccountPasswordError
    end

    local nowTime = os.time()
    local token = jwt.Create({
        user_id = user_data.user_id,
        end_time = nowTime + EXP_TIME
    })

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
    mongo:Update(tabName, {_id = user_data.account}, {
        login_ip = ip,
        gate_id = gate_id,
        login_time = nowTime
    })
    log.Debug("user %s login successful", message.account)
    return XCode.Ok, response
end

return Account