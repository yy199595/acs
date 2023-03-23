
local Main = {}
local Mongo = require("Component.MongoComponent")
local Mysql = require("Component.MysqlComponent")

function Main.Awake()
    print(Mongo, Mysql)
    return true
end

function Main.OnClusterComplete()
    --Mongo.ClearTable("user.account_info")
    local code = Mongo.InsertOnce("user.account_info", {
        _id = "646585122@qq.com",
        user_id = 11223344,
        phone_num = 13716061995,
        register_time = os.time(),
        login_time = os.time(),
        token = "JJIOJOIJOJOO"
    }, 0)
    Log.Error("code = ", code)
    code = Mongo.Update("user.account_info", {
        _id = "646585122@qq.com",
    }, {
        login_time = os.time(),
        token = "112235646546",
        phone_num = 13716061996
    }, 0)
    Log.Error("code = ", code)

   local response = Mongo.QueryOnce("user.account_info", {
        _id = "646585122@qq.com",
    })
   table.print(response)
end
return Main