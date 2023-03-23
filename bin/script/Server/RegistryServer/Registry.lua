local Registry = { }

function Registry.Awake()
    Log.Info("启动注册服务")
    return true
end
function Registry.Regi1ster(request)
    return XCode.Failure
end
return Registry