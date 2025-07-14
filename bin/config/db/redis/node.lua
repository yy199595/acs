local redis_call = redis.call
local str_format = string.format
local json_decode = cjson.decode
local json_encode = cjson.encode
local node_list = "NODE_LIST";

local node = { }

function node.add(request)
    local key = request.id
    local value = json_encode(request)
    redis_call("HSET", node_list, key, value)
end

function node.del(request)

end

function node.list(request)

end

local func = KEYS[1]
local json = ARGV[1]
local request = json_decode(json)
return node[func](request)