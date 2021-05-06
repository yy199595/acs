

NetWorkManager = { }

function NetWorkManager.OnSocketConnect(tcpSession)

    local returnData = { }
    returnData.area_id = 299
    returnData.server_ip = "192.168.0.183"
    returnData.server_port = 1995
    HofixHelper.LoadModuleByName("Manager.NetWorkManager")
    SoEasy.Error("connect new socket ", tcpSession:GetAddress())
    return true
end

function NetWorkManager.OnErrorAfter(tcpSession)
    SoEasy.Warning("remove socket ", tcpSession:GetAddress())
end

return NetWorkManager