
local log = require("Log")
local router = require("net.client")
local Component = require("Component")

local GateComponent = Component()
SetMember(GateComponent, "index", 5000)

function GateComponent:Login(addr, token)
    local fd = router.Connect(addr)
    if fd == -1 then
        log.Error("make client fail addr=%s", addr)
        return nil
    end
    local code = router.Call(fd, "GateSystem.LoginSystem", token)
    if code ~= XCode.Ok then
        log.Error("login to [%s] fail token:(%s)", addr, token)
        return nil
    end
    return fd
end

function GateComponent:Ping(fd)
    local t1 = os.clock() * 1000
    local code = router.Call(fd, "GateSystem.Ping")
    if code ~= XCode.Ok then
        log.Error("call [GateSystem.Ping] code = (%d)", code)
        return
    end
    local t2 = os.clock() * 1000 - t1
    log.Debug("call GateSystem.Ping time = %s ms code=(%d)", t2, code)
end

function GateComponent:Update(fd)
    print("========")
    self:Ping(fd)
    --print(string.format("fd = %d", fd))
end

return GateComponent