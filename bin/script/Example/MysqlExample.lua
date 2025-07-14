local Module = require("Module")

local MysqlExample = Module()

local sqlHelper = require("SqlHelper")

local tab = "user_list"

local userInfo = {
    user_id = 1000,
    open_id = string.range(16),
    name = "xiaoming",
    level = 1,
    attr = {
        hp = 100,
        atk = 200
    },
    create_time = os.time()
}

function MysqlExample:OnStart()
    local table_info = {
        user_id = 4, --四个字节
        open_id = "",
        name = "",
        level = 4,
        attr = { },
        create_time = 8, --八个字节
    }
    local mysql = require("db.mysql")
    print(mysql.run(string.format("DROP TABLE %s;", tab)))
    local crete_sql = sqlHelper.CreateSql(tab, table_info, { "user_id"})

    print(mysql.run(crete_sql))

    local insert_sql = sqlHelper.InsertSql(tab, userInfo)
    print(mysql.run(insert_sql))

    local select_sql = sqlHelper.QuerySql(tab, { user_id = 1000}, {"user_id", "open_id", "attr"}, 1)

    print(mysql.run(select_sql))

    local delete_sql = sqlHelper.DeleteSql(tab, { user_id = 1000 })

    print(mysql.run(delete_sql))

end

function MysqlExample:OnComplete()

    --通过 MysqlDB服务调用
    local mysql = require("MysqlComponent")

    print(mysql:Insert(tab, userInfo))

    print(mysql:Update(tab, { user_id = 1000}, { level = 5}))

    print(mysql:FindOne(tab, { user_id = 1000}, {"user_id", "open_id", "attr"}))

    print(mysql:Delete(tab, { user_id = 1000}))
end


return MysqlExample