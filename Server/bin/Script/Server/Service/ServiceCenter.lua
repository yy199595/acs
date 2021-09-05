ServiceCenter = {}

function ServiceCenter.Add(id, nodeInfo)
    Sentry.Sleep(1000)
    print(type(id), type(nodeInfo))

    return XCode.Successful
end

return ServiceCenter