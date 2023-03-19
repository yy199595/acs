
local Main = {}
local Mongo = MongoComponent
local Mysql = MysqlComponent
function Main.Awake()
    print(Mongo, Mysql)
    return true
end

function Main.OnClusterComplete()
    --Mongo.ClearTable("user.account_info")
end
return Main