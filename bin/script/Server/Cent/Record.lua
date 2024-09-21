
local json = require("util.json")
local RpcService = require("RpcService")
local sqlite = require("SqliteComponent")

local Record = RpcService()

function Record:OnStart()

end

function Record:PushData(request)
    local tab = request.data.tab
    local data = json.decode(request.data.json)
    return sqlite:Insert(tab, data)
end

return Record