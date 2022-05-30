local node = {}

function node.register(request, response) --注册服务
    local json = cjson.encode(request)
    print("json = ", json)
    redis.call("SET", request.address, json)
    redis.call("EXPIRE", request.address, 15)

    local message = cjson.encode({
        eveId = "node_register",
        address = request.address,
        services = request.services
    })
    response.nodes = redis.call("KEYS", "*:*")
    redis.call("PUBLISH", "ServiceMgrComponent", message)
    return true
end

function node.query(request, response)
    assert(type(request.address) == "string")
    response.json = redis.call("GET", request.address)
    return true
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

local func = KEYS[1]
local response = {}
local request = cjson.decode(ARGV[1])
response.res = node[func](request, response)
return cjson.encode(response)