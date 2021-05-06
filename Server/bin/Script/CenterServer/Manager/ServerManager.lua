
ServerManager = { }

function ServerManager.OnSrvRegister(tcpSession, operatorid, logicServerData)

    SoEasy.Error(tcpSession, logicServerData)
    local returnData = { }
    returnData.area_id = 299
    returnData.server_ip = "192.168.0.183"
    returnData.server_port = 1995
    return 3, returnData
end

function ServerManager.OnSrvRegisterTest(tcpSession, operatorid, logicServerData)

    local returnData = { }
    returnData.area_id = 299
    returnData.server_ip = "192.168.0.199"
    returnData.server_port = 199595
    SoEasy.Error(tcpSession, JsonUtil.ToString(logicServerData))
    return 1, returnData
end

function ServerManager.Register(tcpSession, operatorid, data)

    SoEasy.Error(tcpSession, operatorid, JsonUtil.Encode(data))
    return 0, { name = "1122", age = 10 }
end

return ServerManager