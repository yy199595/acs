ServiceCenter = {}

function ServiceCenter.Add(id, nodeInfo)
    print("&&&&&&&&&&&&")
    Sentry.Sleep(1000)
    print(type(id), type(nodeInfo))

    local tab = {}
    tab.name = "小明"
    tab.age = 20

    return XCode.Successful, tab
end

return ServiceCenter