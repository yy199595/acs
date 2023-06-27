

local Main = { }

function Main:Awake()
    local info = {
        account = os.getenv("account"),
        passwd = os.getenv("passwd") or "1234",
        phone_num = tonumber(os.getenv("phone"))
    }
    table.print(info)
    self.player = Class("Player", info)
end

function Main:OnStart()
    self.player:_Invoke("Start")
end

function Main:OnUpdate()
    self.player:_Invoke("OnUpdate")
end


return Main