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
    --SoEasy.BindAction("LoginManager.Login", LoginManager.Login)
    SoEasy.Start(    
        function()
            local queryData =
                MysqlClient.InvokeCommand(
                'shouhuzhemen299_db',
                'select * from player_risk'
            )
            if queryData.code ~= 0 then
                SoEasy.Error(queryData.code, queryData.error)
            else
                for k, v in pairs(queryData.data) do
                    print(k, v)
                end
            end
             print("cost time = ", t2 - t1)
        end
    )
end
