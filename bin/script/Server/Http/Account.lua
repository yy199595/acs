
require("TableUtil")
require("StringUtil")
local log = require("Log")
local jwt = require("Jwt")
local app = require("core.app")
local coroutine_start = coroutine.start

local HttpService = require("HttpService")

local EXP_TIME = 5 * 60
local Account = HttpService()

function Account:Awake()
    self.user_id = 10000
end

function Account:Login(request)

    local message = request.data
    assert(type(message.account) == "string", "user account is not string")
    assert(type(message.password) == "string", "user password is not string")

    self.user_id = self.user_id + 1

    local nowTime = os.time()
    local token = jwt.Create({
        user_id = self.user_id,
        end_time = nowTime + EXP_TIME
    })

    local gate_id =  app.Random("GateSystem")
    local address = app.GetListen(gate_id, "gate")
    if address == nil then
        log.Error("not find gate address {}", gate_id)
        return XCode.AddressAllotFailure
    end

    return XCode.Ok, { token = token, address = address }
end

return Account