local node = {}

function node.add(request, response)
    for _, service in ipairs(request.services) do
        redis.call("SADD", service, request.address)
    end
    return true
end

function node.query(request, response)
    response.services = redis.call("SMEMBERS", request.service)
    return true
end

local has_address = function(array_list, address)
    for _, value in ipairs(array_list) do
        if value == address then
            return true
        end
    end
    return false
end

function node.refresh(request, response)
    response.all_address = {}
    local array_list = redis.call("PUBSUB", "CHANNELS", "*:*")
    local address_list = redis.call("SMEMBERS", request.service)

    for _, address in ipairs(address_list) do
        if has_address(array_list, address) then
            table.insert(response.all_address, address)
        end
    end
    return true
end

function node.remove(request)
    redis.call("SREM", request.service, request.address)
    return true
end

local func = KEYS[1]
local response = {}
local request = cjson.decode(ARGV[1])
response.res = node[func](request, response)
return cjson.encode(response)