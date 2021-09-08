Main = {}
function Main.Awake()
    for key, value in pairs(package.loaded) do
        print(key, value)
    end
end

function Main.Start()
    local registerInfo = {}
    registerInfo.AreaId = 0
    registerInfo.NodeId = 1
    registerInfo.Address = "127.0.0.1:7788"
    Sentry.Call(0, "ServiceCenter", "Add", registerInfo)
end
