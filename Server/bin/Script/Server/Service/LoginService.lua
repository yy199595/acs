
LoginService = {}
local json = require "JsonUtil"
local redisClient = require "RedisClient"
local mysqlClient = require "MysqlClient"
function LoginService.OnInit()
    SoEasy.Warning("init login service")
    return true
end

function LoginService.Register(operId, registerData)

    local accountData = { }
    accountData.userid = 1545646545454
    accountData.passwd = registerData.password
    accountData.account = registerData.account


    local account = registerData.account
    local queryData = redisClient.GetHashValue("tb_player_account", account)
    if queryData ~= nil then
        return 31
    end
    if mysqlClient.Insert("tb_player_account", accountData) then
        local redisCode = redisClient.SetHashValue("tb_player_account", account, accountData)
        SoEasy.Error("redis code", redisCode)
        return 0
    end 
    return 1
end


function LoginService.Query()


end

return LoginService