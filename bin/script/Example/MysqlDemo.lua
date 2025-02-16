local Module = require("Module")

local MysqlDemo = Module()

local log = require("Log")
local guid = require("util.guid")
local sqlHelper = require("SqlHelper")
local ORDER_LIST = "order_list"

local table_info = {
    id = 0,
    city = 0,
    target_id = 0,
    user_id = 0,
    status = 0,
    product_id = 0,
    description = "",
    custom = { },
    create_time = 0,
    amount = 0,
    invite_id = 0,
    icon = ""
}

function MysqlDemo:OnStart()
    print("============")
    local mysql = require("db.mysql")
    local table_sql = sqlHelper.CreateSql(ORDER_LIST, table_info)

    mysql.run(table_sql)

end

function MysqlDemo:OnComplete()

    local mysql = require("MysqlComponent")
    mysql:CreateIndex(ORDER_LIST, "id", true)
    mysql:CreateIndex(ORDER_LIST, "user_id", false)

    table.print(mysql:Exec(sqlHelper.Count(ORDER_LIST)))
    --for i = 1, 1 do
    --    local info = self:RandomData(i)
    --    mysql:Insert(ORDER_LIST, info)
    --end
    local filter = {
        target_id = 3735402380250841092
    }

    local _, res = mysql:Update(ORDER_LIST, filter, {
        custom = {
            table_info, table_info, table_info, table_info
        }
    })
    table.print(res)

    coroutine.start(function()
        table.print(mysql:FindPage(ORDER_LIST, nil, {
            "user_id", "status", "id", "description", "create_time"
        }, 1, 5))
    end)

    coroutine.start(function()
       table.print(mysql:FindPage(ORDER_LIST, nil, {
            "user_id", "status", "id", "description", "create_time"
        }, 2, 5))
    end)
end

function MysqlDemo:RandomData(i)
    local order_id = guid.new()
    return {
        id = order_id,
        city = 5001 + math.random(0, 10),
        target_id = guid.new(start_time, 1),
        user_id = guid.new(start_time, 1),
        status = 1,
        product_id = 100 + i,
        description = time.date(os.time()),
        custom = table_info,
        create_time = os.time(),
        amount = 100,
        invite_id = 0,
        icon = "http://alibab.oss.com/a.icon"
    }
end

return MysqlDemo