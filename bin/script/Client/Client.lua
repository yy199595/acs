local log = require("Log")
local app = require("App")
local json = require("util.json")
local str_format = string.format
local Module = require("Module")
local timer = require("core.timer")
local http = require("HttpComponent")
local redis = require("RedisComponent")

local Main = Module()

Main:SetMember("count", 0)
Main:SetMember("players", { })
Main:SetMember("components", { })
Main:SetMember("city_list", { })

--os.setenv("account", "646585122@qq.com")

local HOST = "http://127.0.0.1:80"
function Main:OnAwake()
    timer.AddUpdate(self, "Update")
end

function Main:Get(url, query, header)
    local str = url
    if query and next(query) then
        str = http:FormatUrl(url, query)
    end
    local response = http:Do("GET", str, header)
    if response == nil or response.body == nil then
        return nil
    end
    if response.body.code ~= XCode.Ok then
        log.Error("[GET][%s] code:%s", str, response.body.error)
        return
    end
    return response.body.data
end

function Main:RandomName()
    local list = { "小明", "小红", "小黄", "小李", "小张"}
    return list[math.random(1, #list)]
end

function Main:RandomIcon()
    local num = math.random(105, 180)
    return str_format("https://game.gtimg.cn/images/yxzj/img201606/heroimg/%s/%s.jpg", num, num)
end

function Main:Post(url, header, body)
    local response = http:Do("POST", url, header, body)
    if response == nil or response.body == nil then
        return nil
    end
    if response.body.code ~= XCode.Ok then
        table.print(response)
        log.Error("[POST][%s] code:%s boyd:%s", url, response.body.error, json.encode(body))
        return
    end
    return response.body.data
end

function Main:StartPlayer(phone)
    local t1 = os.clock() * 1000
    local token = redis:Get(str_format("client:token:%s", phone))
    if token == nil or token == "" then
        local query = { phone = phone }
        local response = self:Get(str_format("%s/user/get_code", HOST), query)
        if response == nil then
            log.Error("获取验证码失败")
            return
        end
        query.code = response.code
        response = self:Post(str_format("%s/user/login", HOST), {}, query)
        if response == nil then
            log.Error("登录失败")
            return
        end
        token = response.token
        redis:Set(str_format("client:token:%s", phone), token, 86400)
    end
    local header = { ["Access-Token"] = token }
    log.Warning("user:%s login ok token = %s", phone, token)

    response = self:Get(str_format("%s/city/list", HOST), {}, header)

    local info = {
        nick = self:RandomName(),
        icon = self:RandomIcon()
    }
    if response and next(response) then
        local max = #response
        local index = math.random(1, max)
        info.city = response[index].code
    end
    self:Post(str_format("%s/user/update", HOST), header, info)
    response = self:Get(str_format("%s/user/find", HOST), nil, header)
    if response == nil then
        log.Error("获取玩家信息失败")
        return
    end

    local user_id = response._id
    local query = { city = 50, page = 1 }
    log.Info("user:%s user_id:%s name:%s", phone, user_id, response.nick)

    response = self:Get(str_format("%s/api/order_list", HOST), {
        page = 1
    })

    if response and next(response) then
        local list = { }
        for _, order_info in ipairs(response) do
            if order_info.order_type == 2 then
                table.insert(list, order_info.product_id)
            end
        end
        self:Post(str_format("%s/user/batch", HOST), header, {
            list = list
        })
    end

    response = self:Get(str_format("%s/api/act_list", HOST), query, header)
    if response == nil then
        log.Error("获取任务列表失败")
        return
    end

    for _, simple_info in ipairs(response) do
        query = { id = simple_info._id }
        local item = self:Get(str_format("%s/api/act_data", HOST), query, header)
        local args = {
            order_type = 2,
            product_id = item._id,
        }
        if item and item.users then
            self:Post(str_format("%s/user/batch", HOST), header, {
                list = item.users
            })
            for _, id in ipairs(item.users) do
                if id == user_id then
                    goto continue
                end
            end
        end
        local result = self:Get(str_format("%s/api/create_order", HOST), args, header)
        if result == nil then
            log.Error("[%s] create order:%s", phone, args.product_id)
        else
            self:Get(str_format("%s/api/pay_done", HOST), {
                id = result.id
            }, header)
        end
        ::continue::
    end
    local t2 = os.clock() * 1000
    log.Error("[%s] user time:(%sms)", user_id, t2 - t1)
end

function Main:OnComplete()
    local phone = 13716061994
    for i = 1, 10 do
        for x = 1, 10 do
            local co = coroutine.create(self.StartPlayer)
            coroutine.resume(co, self, tostring(phone + i))
        end
    end
end

function Main:OnUpdate()

end

function Main:OnSecond()

end

return Main