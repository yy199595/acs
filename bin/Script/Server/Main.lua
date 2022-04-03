Main = {}
function Main.Awake()
   return true
end

function Main.Start()

    Log.Error("star main lua")
    --Sentry.Sleep(1500)

    -- local registerInfo = {}
    -- registerInfo.AreaId = 0
    -- registerInfo.NodeId = 1
    -- registerInfo.Address = "127.0.0.1:7788"
    -- local code, response = GameKeeper.Call(0, "ServiceCenter", "Add", registerInfo)
    -- GameKeeper.Debug("register response ", code, Json.ToString(response))
    return true
end
