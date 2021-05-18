Main = {}
LoginManager = {}
local this = Main
require('Util.JsonUtil')
require('Action.Action')

function Main.Load()
    print(MySqlClient, TcpClientSession)
    local hofix = require('Util.HofixHelper')
end

function LoginManager.Login(session, operId, messageData)
    SoEasy.Sleep(1000)
    print('----------------')
    return 1
end

function Main.Start()
    local table = {}
    table.name = 'yuejianzheng'

    for k, v in pairs(SoEasy) do
        print(k, v)
    end

    --SoEasy.BindAction("LoginManager.Login", LoginManager.Login)
    SoEasy.Start(    
        function()
            local person = { }
            person.Name = "yjz"
            person.Age = 24
           
            local redisClient = require("DataBase.RedisClient")
            local mysqlclient = require("DataBase.MysqlClient")

            local jsonData = mysqlclient.InvokeCommand("shouhuzhemen299_db", "select * from player_risk where roleid=5734")
            if jsonData.code ~= 0 then
                SoEasy.Error(jsonData.error)
            else
                local tableUtil = require("Util.TableUtil")
                --tableUtil.Print(jsonData.data)
            end

            print(redisClient.SetTimeoutValue("yjz_person", person, 1000), "-------------")
            SoEasy.Sleep(500)
            local jsonValue = redisClient.GetValue("yjz_person")
            SoEasy.Info(jsonValue)
        end
    )
end
