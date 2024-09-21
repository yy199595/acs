local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

function Main:Awake()

    self.count = 0
    local request = {fields = {
        name = "xiaohong",
        age = 10,
        list = {
            1122,
            2233,
            3344
        }
    }}
    local json = require("util.json")
    local msg = proto.New("google.protobuf.Struct", json.encode(request))
    print(proto.ToJson(msg))
    --local timer = require("core.timer")
    --timer.AddUpdate(self, "Update")
end

local token = "e30=.eyJ0IjowLCJjIjowLCJ1IjoxMDAwMCwicCI6MTAwfQ==.1q37q2XCTCrRHX9aiax0kzm9CPKo2faBsCaPr/gEP5g="
function Main:Update()
    coroutine.start(function()
        self:DownloadVideo()
    end)
end

function Main:Test()
    local count = 1
    -- local t1 = os.clock() * 1000
    local host = "http://127.0.0.1:8088"
    if token == nil then
        local response = http:Get(string.format("%s/user/get_code", host), {
            phone = "13716061995"
        })

        local code = response.data.code
        response = http:Post(string.format("%s/user/login", host), {
            code = code,
            phone = "13716061995"
        })
        token = response.data.token
    end
    local header = {
        ["Authorization"] = token
    }
    local result = http:Do("GET", string.format("%s/api/act_list?city=%s&page=1", host, 5001), header)
    for _, item in ipairs(result.body.data) do
        local url = string.format("%s/api/act_data?id=%s", host, item._id)
        http:Do("GET", url, header)
    end
    count = count + #result.body.data
    -- log.Error("[count=%s] cost time = %s", count + 2, os.clock() * 1000 - t1)
end

function Main:OnComplete()

end

function Main:DownloadVideo()
    local t1 = os.clock()
    self.count = self.count + 1
    local app = require("App")
    local log = require("Log")
    local json = require("util.json")
    local response = http:Do("GET", "http://api.yujn.cn/api/zzxjj.php?type=image")
    local data = json.decode(response.body)
    if data == nil then
        log.Error("%s", response.body)
        return
    end
    local address, title = data.url, data.title
    if address and #address > 0 then
        local uid = app:NewGuid()
        http:Download(address, string.format("%s/mp4/%s.mp4", os.dir, title))
    end
    local t2 = os.clock() - t1
    log.Info("[%s] 下载视频成功 耗时=> [%s]ms", self.count, t2)
end

function Main:Download()
    for i = 105, 180 do
        local t1 = os.clock() * 1000
        local path = string.format("%s/image/%s.jpg", os.dir, i)
        local res = http:Download(string.format("https://game.gtimg.cn/images/yxzj/img201606/heroimg/%s/%s.jpg", i, i),
            path)
        if res ~= nil then
            local t = math.ceil(os.clock() * 1000 - t1)
            print(string.format("[%sms] download %s.jpg ok count:%s", t, i, i - 104))
        end
    end
end

return Main
