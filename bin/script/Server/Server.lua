local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")

local Main = Module()

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

function Main:Awake()
    self.count = 0
    local timer = require("core.timer")
    -- timer.AddUpdate(self, "Update")
end

function Main:Update()
    for i = 1, 10 do
        coroutine.start(function()
            self:Test()
        end)
    end
end

function Main:Test()
    local count = 1
    -- local t1 = os.clock() * 1000
    local host = "http://101.34.67.141:80"
    local response = http:Get(string.format("%s/user/get_code", host), {
        phone = "13716061995"
    })
    local code = response.data.code
    response = http:Post(string.format("%s/user/login", host), {
        code = code,
        phone = "13716061995"
    })
    local token = response.data.token
    local header = {
        ["Access-Token"] = token
    }
    local result = http:Do("GET", string.format("%s/api/act_list?city=%s", host, 50), header)
    for _, item in ipairs(result.body.data) do
        local url = string.format("%s/api/act_data?id=%s", host, item._id)
        http:Do("GET", url, header)
    end
    count = count + #result.body.data
    -- log.Error("[count=%s] cost time = %s", count + 2, os.clock() * 1000 - t1)
end

function Main:GetZeroTime(days)
    local count = days or 0
    local now_time = os.time() + count * 86400
    local current_date = os.date("*t", now_time)
    local year = current_date.year
    local month = current_date.month
    local day = current_date.day

    return os.time({
        year = year,
        month = month,
        day = day,
        hour = 0,
        min = 0,
        sec = 0
    })
end

function Main:GetMemberCount(city)
    local count1 = mongo:Count("user_info_list", {
        card_id = 1
    })
    local count2 = mongo:Count("user_info_list", {
        card_id = 2
    })
    print(string.format("年费会员:%s  永久会员:%s", count1, count2))
end

function Main:OnComplete()

    --self:GetMemberCount()
    --local amount = 0
    --local response = mongo:Find("order_list", { product_id = 315},nil, 20)
    --for _, info in ipairs(response) do
    --    amount = amount + info.price
    --end
    --print(string.format("%.2f", amount / 100))
    --local page = 0
    --local amount = 0
    --local userId = 10004
    --while true do
    --    page = page + 1
    --    local response = mongo:FindPage("user_wallet", { user_id = userId}, page, 30)
    --    if response == nil or #response == 0 then
    --        break
    --    end
    --    for _, info in ipairs(response) do
    --        amount = amount + info.amount
    --    end
    --end
    --print(string.format("金额:%.2f", amount / 100))
    --mongo:Update("user_info_list", { user_id = userId}, {
    --    amount = amount
    --})

    --local response = http:Get("http://127.0.0.1:8088/order_mgr/filter", {
    --    city = 3601,
    --    start = 1717171200,
    --    stop = 1718467200
    --})

    --local response = mongo:Find("order_list", {
    --    create_time = {
    --        ["$gte"] = self:GetZeroTime(-1),
    --        ["$lte"] = self:GetZeroTime(0)
    --    },
    --    city = 4201
    --})
    --table.print(response)
end

function Main:OnStart()

    -- redis:ClearAll()
    -- mongo:Drop("club_list")
    -- mongo:Drop("user_info_list")
    -- mongo:Drop("club_member_list")
    -- mongo:Drop("user_info_list")
    -- mongo:Drop("activity_list")
    -- mongo:Drop("order_list")
    -- mongo:Drop("user_coupon_list")
    -- mongo:Drop("user_coupon_list")

    -- mongo:Update("club_list", { _id = 1001 }, {
    --    permission = 10, status = 0
    -- })
    -- mongo:Drop("club_member_list")
    -- local page = 1
    -- while true do
    --    local response = mongo:FindPage("activity_list", {}, page, 10)
    --    if response == nil or (not next(response)) then
    --        break
    --    end
    --    for _, info in ipairs(response) do
    --        local urls = {}
    --        local urlRegex = '"(http[s]?://%S+)"'
    --        for url in info.content:gmatch(urlRegex) do
    --            table.insert(urls, url)
    --        end
    --        mongo:Update("activity_list", { _id = info._id}, {
    --            urls = urls
    --        })
    --    end
    --    page = page + 1
    -- end
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
