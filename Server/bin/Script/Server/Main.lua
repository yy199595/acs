Main = {}
function Main.Awake()
   
end

function Main.Start()
    
    print("---------",coroutine.running())
    
    local registerInfo = {}
    registerInfo.AreaId = 0
    registerInfo.NodeId = 1
    registerInfo.Address = "127.0.0.1:7788"
    local code, response = Sentry.Call(0, "ServiceCenter", "Add", registerInfo)
    print("**********", code, TableUtil.Print(response))
end
