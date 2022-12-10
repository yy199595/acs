
local Main = {}
local Mongo = require("Component.MongoComponent")
local Mysql = require("Component.MysqlComponent")
function Main.Awake()
    return true
end

function Main.OnClusterComplete()
    --Mongo.ClearTable("user.account_info")
end
return Main
