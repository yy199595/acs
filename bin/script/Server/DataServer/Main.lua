
local DataServer = { }
DataServer.Start = function()

end
local function CreateTable(tabName, keys)
    local code = App.Call("MysqlService.Create", {
        keys = keys,
        data = Proto.New(tabName)
    })
    return code == XCode.Successful
end

function DataServer.OnLocalComplete()
    --CreateTable("user.account_info", {"account"})
end
return DataServer