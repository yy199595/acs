local Module = require("Module")

local PgsqlExample = Module()

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

function PgsqlExample:OnStart()
    local table_info = {
        user_id = 4, --四个字节
        open_id = "",
        name = "",
        level = 4,
        attr = { },
        create_time = 8, --八个字节
    }
    local pgsql = require("db.pgsql")
    print(pgsql.run(string.format("DROP TABLE %s;", tab)))
    local crete_sql = sqlHelper.CreateSql(tab, table_info)

    print(pgsql.run(crete_sql))

    local insert_sql = sqlHelper.InsertSql(tab, userInfo)
    print(pgsql.run(insert_sql))

    local select_sql = sqlHelper.QuerySql(tab, { user_id = 1000}, {"user_id", "open_id", "attr"}, 1)

    print(pgsql.run(select_sql))

    local delete_sql = sqlHelper.DeleteSql(tab, { user_id = 1000 })

    print(pgsql.run(delete_sql))

end

function PgsqlExample:OnComplete()

    --通过 pgsqlDB服务调用
    local pgsql = require("PgsqlComponent")

    print(pgsql:Insert(tab, userInfo))

    print(pgsql:Update(tab, { user_id = 1000}, { level = 5}))

    print(pgsql:FindOne(tab, { user_id = 1000}, {"user_id", "open_id", "attr"}))

    print(pgsql:Delete(tab, { user_id = 1000}))
end


return PgsqlExample