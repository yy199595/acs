
NetWorkManager = { }
local this = NetWorkManager
NetWorkManager.mSessionArray = { }
function NetWorkManager.OnSocketConnect(tcpSession)

    local returnData = { }
    returnData.area_id = 299
    returnData.server_ip = "192.168.0.183"
    returnData.server_port = 1995

    SoEasy.Error(pApplocation, pApplocation:GetFps(), pApplocation:GetServerName())
    return true
end
local timerId = 0
function NetWorkManager.OnConnectAfter(tcpSession, code)

    SoEasy.Error(tcpSession, code)
    if code == false then
        SoEasy.Info("connect ", tcpSession, " Fail")
        return
    end
    timerId = SoEasy.AddTimer(NetWorkManager.OnUpdate, 50, -1)
    table.insert(this.mSessionArray, tcpSession:GetAddress())
    print(timerId)
end

function NetWorkManager.OnUpdate()
    for i, v in ipairs(this.mSessionArray) do
        local address = this.mSessionArray[i]
        local tcpSession = luaNetWorkManager:GetSessionByAdress(address)
        
        local returnData = {}
        returnData.area_id = 299
        returnData.server_ip = '192.168.0.183'
        returnData.server_port = 1995
        local t1 = TimeHelper.GetMilTimestamp()
        local requestData = SoEasy.CreateByTable('SrvProtocol.LogicServerData', returnData)

        SoEasy.Call(tcpSession, "ServerManager.OnSrvRegisterTest", requestData, function(code, message)
           
        end)
    end
end


return NetWorkManager
