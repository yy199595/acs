
local Main = {}
local MongoDB = require("Server.MongoComponent")
local redis = require("Server.RedisComponent")
function Main.Awake()

    local t1 = Time.NowMilTime()
    redis.SyncRun("SET", "234", 12345)
    redis.SyncRun("SET", "345", 12345)
    local res = redis.SyncRun("KEYS *")
    table.print(res)
    print("use time = ", Time.NowMilTime() - t1)

     --table.print(response)

    --local id = Sqlite.Open("server")
    --local res = Sqlite.Query(id, "select * from registry")
    --local res1 = Sqlite.QueryOnce(id, "select * from registry")
    --table.print(res)
    --table.print(res1)
    return true
end


function Main.OnClusterComplete()
    --MongoDB.ClearTable("user.account_info")
    --Http.Download("http://127.0.0.1:8080/1122.exe", "D:\\trunk\\ssh\\Sentry\\bin\\1122.exe");
   
    --local id = Mysql.Make()
    --local res = Mysql.Exec(id, "insert into server.registry(server_name,rpc_address,http_address)values('test','127.0.0.1:7788','http://127.0.0.1:80')")
    --res = Mysql.QueryOnce(id, "select * from server.registry")
    --table.print(res)
    local code = MongoDB.InsertOnce("user.account_info", {
        _id = "646585122@qq.com",
        user_id = 11223344,
        phone_num = 13716061995,
        register_time = os.time(),
        login_time = os.time(),
        token = "JJIOJOIJOJOO"
    }, 0)
    Log.Error("code = ", code)
    code = MongoDB.Update("user.account_info", {
        _id = "646585122@qq.com",
    }, {
        login_time = os.time(),
        token = "112235646546",
        phone_num = 13716061996
    }, 0)
    Log.Error("code = ", code)

   local response = MongoDB.QueryOnce("user.account_info", {
        _id = "646585122@qq.com",
    })
   table.print(response)
end
return Main