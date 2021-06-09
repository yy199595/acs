LoginService = {}
local redisClient = require "RedisClient"
local mysqlClient = require "MysqlClient"
function LoginService.OnInit()
    SoEasy.Warning("init login service")
    return true
end

function LoginService.Register(_, registerData)
    local accountData = {}
    accountData.userid = 1545646545454
    accountData.passwd = registerData.password
    accountData.account = registerData.account

    local account = registerData.account
    local queryData = redisClient.GetHashValue("tb_player_account", account)
    SoEasy.Info(queryData)
    if queryData ~= nil then
        return XCode.AccountAlreadyExists
    end
    local res = mysqlClient.Insert("tb_player_account", accountData)
    print(res)
    if res then
        local redisCode = redisClient.SetHashValue("tb_player_account", account, accountData)
        SoEasy.Error("redis code", redisCode)
        return XCode.Successful
    end
    return XCode.Failure
end

function LoginService.Query()
end

return LoginService
