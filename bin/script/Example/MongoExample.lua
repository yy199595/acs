local Module = require("Module")

local MongoExample = Module()

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


function MongoExample:OnStart()

    userInfo._id = userInfo.user_id
    local mongo = require("MongoComponent")

    print(mongo:SetIndex(tab, "user_id", 1, true))

    print(mongo:InsertOnce(tab, userInfo))

    local filter = { user_id = userInfo.user_id}

    print(mongo:UpdateOne(tab, filter, {
        level = 2
    }))

    print(mongo:FindOne(tab, filter))

    print(mongo:Delete(tab, filter))
end

function MongoExample:OnComplete()

end


return MongoExample