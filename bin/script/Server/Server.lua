
local Main = {}
local Mongo = require("Component.MongoComponent")

function Main.Awake()

    --local id = Sqlite.Open("server")
    --local res = Sqlite.Query(id, "select * from registry")
    --local res1 = Sqlite.QueryOnce(id, "select * from registry")
    --table.print(res)
    --table.print(res1)
    return true
end


function Main.OnClusterComplete()
    
    --Mongo.ClearTable("user.account_info")
    --Mysql.Create({
    --    name = "server.registry",
    --    fields = {
    --        server_name = "",
    --        rpc_address = "",
    --        http_address = "",
    --        last_ping_time = 0,
    --    },
    --    keys = {
    --        "rpc_address"
    --    },
    --    index = {
    --        "rpc_address"
    --    }
    --})

    local id = Mysql.Make()
    local res = Mysql.Exec(id, "insert into server.registry(server_name,rpc_address,http_address)values('test','127.0.0.1:7788','http://127.0.0.1:80')")
    res = Mysql.QueryOnce(id, "select * from server.registry")
    table.print(res)
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