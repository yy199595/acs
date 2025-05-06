local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()

SetMember(Main, "count", 1)

local timer = require("core.timer")

local timerId
local count = 0
function Main:OnAwake()
    timer.timeout(1000, function()
        print("一秒")
    end)

    timer.timeout(1000 * 60, function()
        print("一分钟")
    end)

    timer.timeout(1000 * 60 * 60, function()
        print("一小时")
    end)

    timer.timeout(1000 * 60 * 60 * 24, function()
        print("一天")
    end)

    --for i = 1, 100000 do
    --    timer.timeout(i * 1000, self, "OnFrameUpdate")
    --end
end

function Main:OnFrameUpdate()
    count = count + 1
    print("count => ", count, os.date())
end

function Main:OnStart()

end

local get_format_time = function(t)
    return tonumber(os.date("%Y%m%d", t))
end

function Main:OnComplete()

    --local info = { }
    --local page = 0
    --while true do
    --    page = page + 1
    --    local response = mongo:FindPage("yjz.user_info_list", nil, page, 500, { "create_time"}, {
    --        create_time = 1
    --    })
    --    if response == nil or #response == 0 then
    --        break
    --    end
    --    for _, userInfo in ipairs(response) do
    --        local create_time = get_format_time(userInfo.create_time)
    --        if info[create_time] == nil then
    --            info[create_time] = 0
    --        end
    --        info[create_time] = info[create_time] + 1
    --    end
    --end
    --local timerInfo = { }
    --for i, v in pairs(info) do
    --    table.insert(timerInfo, {
    --        day = i,
    --        num = v
    --    })
    --end
    --
    --table.sort(timerInfo, function(a, b)
    --    return a.day < b.day
    --end)
    --
    --table.print(timerInfo)
end

function Main:OnUpdate()

end

return Main
