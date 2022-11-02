
local user = {}
local channel = "UserSyncComponent"
user.add_user = function(request)
    if redis.call("SADD", "user_account", request.account) == 0 then
        return 0
    end
    return redis.call("INCR", "user_id") + 1995;
end

user.set_token = function(request)
    redis.call("SET", request.token, request.user_id)
    redis.call("EXPIRE", request.token, request.time)
    return true
end

user.get_token = function(request)
    local user_id = redis.call("GET", request.token)
    if user_id ~= nil then
        print(type(user_id), tonumber(user_id))
        redis.call("DEL", request.token)
        return tonumber(user_id)
    end
    return 0
end

user.set_state = function(request)
    if request.state == 0 then --离线
        local data = {}
        data.eveId = "user_exit_event"
        data.user_id = request.user_id
        local key = tostring(request.user_id)
        data.services = redis.call("HEGTALL", key)
        redis.call(channel, cjson.decode(data))
    end
    redis.call("HSET", "user_state", request.user_id, request.state)
    return true
end

user.get_state = function(request)
    return redis.call("HGET", "user_state", request.user_id)
end

user.set_address = function(request)

    request.eveId = "user_join_event"
    request.user_id = request.user_id
    request.address = request.address
    request.service = request.service
    local key = tostring(request.user_id)
    redis.call("HSET", key, request.service, request.address)

    if request.broadcast == true then --广播给所有的服务
        redis.call("PUBLISH", request.service, cjson.encode(request))
    else  -- 只通知目标服务
        redis.call("PUBLISH", request.address, cjson.encode(request))
    end
    return true
end

user.get_address = function(request)
    local state = redis.call("HGET", "user_state", request.user_id)
    if tonumber(state) >= 1 then
        local key = tostring(request.user_id)
        return redis.call("HGET", key, request.service)
    end
end

local result = { }
local method = user[KEYS[1]]
if type(method) ~= "function" then
    result.error = KEYS[1] .. " not lua function"
else
    local request = cjson.decode(ARGV[1])
    local state, err, response = pcall(method, request)
    if not state then
        result.error = err
    else
        result.data = response
    end
end
return cjson.encode(result)