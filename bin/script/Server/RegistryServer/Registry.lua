

local Registry = { }
local registers = { }
require("XCode")
require("TableUtil")
local tab_name = "server.registry"
local mysql = require("Server.MysqlClient")
local nodeService = require("Service").New("Node")
function Registry.Awake()
    Proto.Import("mysql/server.proto")
    return mysql.NewTable(0, tab_name, {
        pb = "server.registry",
        keys = { "server_id" },
        fields = { }
    })
end

function Registry.Register(request)
    local address = request.from
    local message = request.message

    local data = {
        last_ping_time = os.time(),
        server_id = message.server_id,
        server_name = message.server_name,
        rpc_address = message.listens.rpc,
        http_address = message.listens.http or "",
        gate_address = message.listens.gate or ""
    }
    if not mysql.Replace(0, tab_name, data) then
        return XCode.SaveToMysqlFailure
    end
    registers[address] = message
    for target, _ in ipairs(registers) do
        Service.Send(target, "Node.Join", message)
    end
    return XCode.Successful
end

function Registry.Query(request)
    table.print(request)
    local result = nil
    local name = request.message.server_name
    if #name == 0 then
        result = mysql.Query(0, tab_name)
    else
        result = mysql.Query(0, tab_name, nil, {
            server_name = name
        })
    end
    if result == nil then
        return XCode.MysqlResultIsNull
    end
    local response = { }
    response.list = { }
    for _, info in ipairs(result) do
        table.insert(response.list, {
            server_id = info.server_id,
            server_name = info.server_name,
            listens = {
                rpc = info.rpc_address,
                http = info.http_address,
                gate = info.gate_address
            }
        })
    end
    return XCode.Successful, response
end

function Registry.Ping(request)
    local address = request.from
    local info = registers[address]
    if info == nil then
        return XCode.Failure
    end
    mysql.Update(0, tab_name, { last_ping_time = os.time() }, {
        server_id = info.server_id
    })
    return XCode.Successful
end

function Registry.UnRegister(request)
    local address = request.from
    local info = registers[address]
    if info == nil then
        return XCode.Failure
    end
    registers[address] = nil
    mysql.Delete(0, tab_name, { server_id = info.server_id })
    return XCode.Successful
end

return Registry