local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

function Main:Awake()
	
end

function Main:OnComplete()
    local oss = require("util.oss")
    local url = oss.Upload("/Users/yy/Desktop/acs/Common/Timer/Lua/Timer.cpp", "10000")
    print(url)
end

return Main
