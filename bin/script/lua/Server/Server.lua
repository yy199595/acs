
local Server = {}
Server.Modules = { }
local Mongo = require("Component.MongoComponent")

function GetModules()
    local modules = { }
    for _, module in pairs(package.loaded) do
        if type(module) == "table" then
            table.insert(modules, module)
        end
    end
    return modules
end

function Server.Awake()
    table.print(Http)
    return true
end

function Server.StartInsert()
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

function Server.OnClusterComplete()
    local code, data = Http.Get("http://www.kuaidi100.com/query?type=1122&postid=qwer")
    print(code, data)
    --MysqlComponent.Create("user.account_info", {"account"})
    --
    --MysqlComponent.Add("user.account_info", {
    --    account = "646585122@qq.com",
    --    user_id = 199595,
    --    phone_num = 13716061995,
    --    pass_word = "199595yjz.",
    --    register_time = os.time(),
    --    last_login_ip = "127.0.0.1",
    --})

    --local res = MysqlComponent.QueryFields("user.account_info", {"user_id", "phone_num"}, {
    --    account = "646585122@qq.com"
    --}, 1)
    --table.print(res)
    
    local r1 = Mongo.InsertOnce("user.data_account", {
        _id = "646585122@qq.com",
        login_ip = "127.0.0.1",
        user_id = 1122,
        login_time = os.time(),
        register_time = os.time(),
        token = "0x00ssdjsaklj"
    })

    --MysqlComponent.Update("user.account_info", {
    --    account = "646585122@qq.com"
    --}, {
    --    user_area_list = {
    --        list = { 10, 11, 12, 13, 14}
    --    }
    --})
    --
    --local res = MysqlComponent.QueryOnce("user.account_info", {
    --    account = "646585122@qq.com"
    --})
    --print("======== text query from mysql ========")
    --table.print(res)


    local r3 = Mongo.InsertOnce("user2.data_account", {
        _id = "646585123@qq.com",
        login_ip = "127.0.0.1",
        user_id = 1122,
        login_time = os.time(),
        register_time = os.time(),
        token = "0x00ssdjsaklj"
    })

    local r2 = Mongo.Update("user1.data_account", {
        _id = "646585122@qq.com"
    }, {
        login_time_1 = 199595
    })

    Mongo.Delete("user2.data_account", {
        _id = "646585122@qq.com"
    })

    local tab = { }
    table.insert(tab, "646585123@qq.com")
    table.insert(tab, "646585122@qq.com")
    local response = Mongo.QueryDatas("user1.data_account", tab)
    --local response = MongoComponent.Query("data_account", {
    --    _id = {
    --        ["$in"] = {
    --            "646585123@qq.com",
    --            "646585122@qq.com"
    --        }
    --    }
    --})
    print("======== text query from mongo ========")
    table.print(response)
end

function Server.Hotfix()
    local modules = GetModules()
    for _, module in pairs(modules) do
        local method = module["Hotfix"]
        if type(method) == "function" then
            local state, err = pcall(method)
            if not state then
                Log.Error(err)
            end
        end
    end
end

return Server
