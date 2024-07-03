local table_insert = table.insert

local app = require("App")
local jtw = require("auth.jwt")
local USER_LIST = "user_info_list"
local ORDER_LIST = "order_list"
local ACTIVITY_LIST = "activity_list"
local mongo = require("MongoComponent")
local HttpService = require("HttpService")

local AccountManager = HttpService()

function AccountManager:List(request)
    local count = 0
    local response = { }
    local user_id = request.query.open_id
    local fields = { "_id", "user_id", "nick", "sex", "icon", "desc", "permission",
                      "create_time", "city_name", "card_id", "vip_time" }
    if user_id ~= nil and user_id ~= "" then
        local result = mongo:FindOne(USER_LIST, { user_id = tonumber(user_id) }, fields)
        if result == nil then
            result = mongo:FindOne(USER_LIST, { _id = user_id}, fields)
        end
        if result ~= nil then
            count = 1
            table_insert(response, result)
        end
    else
        local filter = { }
        if request.query.city_id then
            local city_id = tonumber(request.query.city_id)
            if city_id and city_id > 0 then
                filter.city = city_id
            end
        end
        local page = tonumber(request.query.page)
        if page == nil then
            return XCode.CallArgsError
        end
        count = mongo:Count(USER_LIST, filter)
        response = mongo:FindPage(USER_LIST, filter, page, 10, fields, {
            user_id = -1
        })
    end
    return XCode.Ok, { count = count, list = response }
end

function AccountManager:Change(request)
    local user_id = request.data.user_id
    return mongo:Update(USER_LIST, { user_id = user_id}, {
        sex = request.data.sex,
        nick = request.data.nick,
        permission = request.data.permission
    })
end

function AccountManager:Find(request)
    local user_id = request.query.id
    if user_id == nil or user_id == "" then
        return XCode.CallArgsError
    end
    local filter = {
        user_id = tonumber(user_id)
    }
    local response = mongo:FindOne(USER_LIST, filter)
    if response == nil then
        return XCode.NotFoundData
    end
    if response.activity_list and #response.activity_list > 0 then
        local fields = { "_id", "title", "address", "status", "start_time", "stop_time" }
        local activity_list = mongo:FindWhere(ACTIVITY_LIST, response.activity_list, fields)
        if activity_list ~= nil and #activity_list > 0 then
            response.activity_list = activity_list
        end
    end
    local order_list = mongo:Find(ORDER_LIST, filter, { "_id", "type", "status",
                                                        "price", "product_id", "desc", "create_time" })
    if order_list ~= nil and #order_list > 0 then
        response.order_list = order_list
    end
    return XCode.Ok, response
end

function AccountManager:Delete(request)
    local open_id = request.query.open_id
    return mongo:Delete(USER_LIST, { _id = open_id })
end

function AccountManager:GenToken(request)
    local id = request.query.id
    if id == nil or id == "" then
        return XCode.CallArgsError
    end
    local conf = app:GetConfig("core")
    local token = jtw.Create(conf.secret, {
        p = 1,
        u = tonumber(id),
        t = os.time() + 60 * 60
    })
    return XCode.Ok, { token = token }
end

return AccountManager