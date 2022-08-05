local node = {}

function node.set(key, value, time)

    redis.call("SET", key, cjson.encode(value))
    redis.call("EXPIRE", key, time)
end

local check_services = function(address)
    local services = { }
    if type(address) == "string" then
        if redis.call("EXISTS", address) then
            local str = redis.call("GET", address)
            services[address] = cjson.decode(str)
        else
            redis.call("SREM", "address", address)
            print("remove address = [", address, "]")
        end
    else
        local addressInfos = redis.call("SMEMBERS", "address")
        if type(addressInfos) ~= "table" then
            return false
        end
        for _, key in ipairs(addressInfos) do
            if redis.call("EXISTS", key) then
                local str = redis.call("GET", key)
                print("str = ", str)
                services[key] = cjson.decode(str)
            else
                redis.call("SREM", "address", key)
                print("remove address = [", key, "]")
            end
        end
    end
    return services
end

function node.update(request, response) --更新服务

    redis.call("SADD", "address", request.address)
    node.set(request.address, request.services, 15)

    if request.broacast then
        local data = {
            t = 10,
            address = request.address,
            services = request.services
        }
        local message = cjson.encode(data)
        response.services = check_services()
        redis.call("PUBLISH", "ServiceMgrComponent.OnNodeRegister", message)
    end
    return true
end

function node.query(request, response)
    if type(request.address) == "string" then
        return redis.call("EXISTS", request.address) == 1
    end
    return false
end

function node.refresh(request, response)
    redis.call("EXPIRE", request.address, 15)
    response.nodes = redis.call("KEYS", "*:*")
    return true
end

function node.add(request) --添加一个新服务
    local json = redis.call("GET", request.address)
    local nodeInfo = cjson.decode(json)
    table.insert(nodeInfo.services, request.address)
    redis.call("SET", request.address, nodeInfo)
    local message =  {
        eveId = "service_add",
        address = request.address,
        service = request.service
    }
    redis.call("PUBLISH", "ServiceMgrComponent", cjson.encode(message))
end

function node.del(request)
    local json = redis.call("GET", request.address)
    local nodeInfo = cjson.decode(json)
    for index = 1, #nodeInfo.services do
        if nodeInfo.services[index] == request.service then
            table.remove(nodeInfo.services, index)
            break
        end
    end
    redis.call("SET", request.address, cjson.encode(nodeInfo))

    local message =  {
        eveId = "service_del",
        address = request.address,
        service = request.service
    }
    redis.call("PUBLISH", "ServiceMgrComponent", cjson.encode(message))
end

print(KEYS[1], ARGV[1])
local func = KEYS[1]
local response = {}
local request = cjson.decode(ARGV[1])
response.res = node[func](request, response)
return cjson.encode(response)