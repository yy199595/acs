
local Command = {}

Command.Service = {}

Command.Service.Push = function(keys)
    assert(type(keys) == 'table')
    local count = 0
    local key = "rpc.services"
    print(table.concat(keys, ","))
    for _, name in ipairs(keys) do
       count = count + redis.call('SADD', key, name)
    end
    return count
end

Command.Service.Add = function(keys)
    local service = keys[1]
    local address = keys[2]
    print(string.format("add new service [%s.%s]", service, address))
    return redis.call('SADD', string.format("rpc.%s", service), address)
end

Command.Service.Get = function(service)
    local key = string.format("rpc.%s", service)
    return redis.call('SINTER', key)
end

Command.Service.Remove = function(keys)
    local count = 0
    local address = keys[1]
    local services = redis.call('SMEMBERS','rpc.services')
    for _, name in ipairs(services) do
        local key = string.format("rpc.%s", name)
        count = count + redis.call('SREM', key)
    end
    print("remove ", address, " count = ", count)
    return count
end


Command.Call = function(key1, key2, args)
    assert(type(Command[key1]) == 'table')
    assert(type(Command[key1][key2]) == 'function')
    return Command[key1][key2](args)
end



if #KEYS < 2 then
    return "args number min = 2"
end

print("call function ", KEYS[1], KEYS[2])

local args = {}
for index = 3, #KEYS do
    table.insert(args, KEYS[index])
end

return Command.Call(KEYS[1], KEYS[2], args)