Main = {}
function Main.Awake()
   
end

function Main.Start()
    
    Sentry.Debug("+++++",coroutine.running())
    
    local registerInfo = {}
    registerInfo.AreaId = 0
    registerInfo.NodeId = 1
    registerInfo.Address = "127.0.0.1:7788"
    local code, response = Sentry.Call(0, "ServiceCenter", "Add", registerInfo)
    Sentry.Debug("register response ", code, Json.ToString(response))
end
