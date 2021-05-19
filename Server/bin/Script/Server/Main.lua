Main = {}
local this = Main
require('Util.JsonUtil')
require('Action.Action')

function Main.Load()
    local hofix = require('Util.HofixHelper')
end

function Main.Start()
    local cor =
        coroutine.create(
        function()
            local person = {}
            person.Name = 'yjz'
            person.Age = 24

            local redisClient = require('DataBase.RedisClient')
            local mysqlclient = require('DataBase.MysqlClient')

            local user = {}
            user.account = '646585122@qq.com'
            user.platform = 'ios_wechat'
            user.userid = 156465465478748456
            user.passwd = '199595yjz.'
            --user.registertime = os.time()

            print(mysqlclient.Insert('yjz', 'tb_player_account', user))

            print(redisClient.SetTimeoutValue('yjz_person', person, 1000), '-------------')
            SoEasy.Sleep(500)
            local jsonValue = redisClient.GetValue('yjz_person')
            SoEasy.Info(jsonValue)
        end
    )
    coroutine.resume(cor)
end
