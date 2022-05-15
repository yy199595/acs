
local user = {}
user.add_user = function(request, response)
    if redis.call("SADD", "user_account", request.account) == 0 then
        return false
    end
    response.user_id = redis.call("INCR", "user_id") + 1995;
   return true
end

user.set_token = function(request, response)
    redis.call("SET", request.token, request.user_id)
    redis.call("EXPIRE", request.token, request.time)
    return true
end

user.get_token = function(request, response)
    local user_id = redis.call("GET", request.token)
    print(type(user_id), tonumber(user_id))
    if user_id ~= nil then
        redis.call("DEL", request.token)
        response.user_id = tonumber(user_id)
        return true
    end
    return false
end

user.set_state = function(request, response)
    if request.state <= 0 then --离线
        local data = {}
        data.eveId = "user_exit_event"
        data.user_id = request.user_id
        local key = tostring(request.user_id)
        local services = redis.call("HEGTALL", key)
        local json = cjson.decode(data)
        for service, _ in ipairs(services) do
            redis.call("PUBLISH", service, json)
        end
    end
    redis.call("HSET", "user_state", request.user_id, request.state)
    return true
end

user.get_state = function(request, response)
    response.state = redis.call("HGET", "user_state", request.user_id)
    return true
end

user.set_address = function(request, response)

    request.eveId = "user_join_event"
    request.user_id = request.user_id
    request.address = request.address
    request.service = request.service
    local key = tostring(request.user_id)
    redis.call("HSET", key, request.service, request.address)
    redis.call("PUBLISH", request.service, cjson.encode(request))
    return true
end

user.get_address = function(request, response)
    local state = redis.call("HGET", "user_state", request.user_id)
    if tonumber(state) >= 1 then
        local key = tostring(request.user_id)
        response.address = redis.call("HGET", key, request.service)
        return true
    end
    return false
end

local func = KEYS[1]
local response = {}
local request = cjson.decode(ARGV[1])
print(func, "request = ", ARGV[1])
response.res = user[func](request, response)
local responseJson = cjson.encode(response)
print("response = ", responseJson)
return responseJson