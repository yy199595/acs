local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()

SetMember(Main, "count", 1)

function Main:OnAwake()
end

function Main:OnStart()

end

function Main:OnComplete()

end

function Main:OnUpdate()

end

return Main
