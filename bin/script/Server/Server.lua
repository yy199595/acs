
local Main = {}
local Mongo = require("Component.MongoComponent")

function Main.Awake()
    return true
end

function Main.StartInsert()
    while true do
        MysqlComponent.Add("user.account_info", {
        account = "646585122@qq.com",
        user_id = 199595,
        phone_num = 13716061995,
        pass_word = "199595yjz.",
        register_time = os.time(),
        last_login_ip = "127.0.0.1",
        user_area_list = {
            list = {1, 2, 3, 4}
        },
        }, 1)

        local res = MysqlComponent.QueryOnce("user.account_info", {
            account = "646585122@qq.com"
        })
        table.print(res)
    end
end

function Main.OnClusterComplete()
    Mongo.ClearTable("user.account_info")
end
return Main
