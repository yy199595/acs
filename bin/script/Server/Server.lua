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
    --table.print(oss.Sign({
    --    file_name = "zhongqiuhaibao",
    --    file_type = "image/jpg",
    --    max_length = 1024 * 1024 * 5,
    --    expiration = os.time() + 60 * 5,
    --    limit_type = { "image/jpg" },
    --    upload_dir = "10000/"
    --}))
    --local url = oss.Upload("/Users/yy/Desktop/image/zhongqiuhaibao.jpg", "10000")
    --print(url)
end

return Main
