
ServerManager = { }

function ServerManager.OnSrvRegister(tcpSession, operatorid, logicServerData)

    SoEasy.Log(tcpSession, operatorid,logicServerData)
    
    local returnData = { }
    returnData.area_id = 299
    returnData.server_ip = "192.168.0.183"
    returnData.server_port = 1995

    

    print(returnData)

    return 5, returnData
end

return ServerManager
