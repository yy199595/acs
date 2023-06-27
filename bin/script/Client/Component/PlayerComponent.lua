local PlayerComponent = { }

function PlayerComponent:OnInit()
    self.players = { }
    self.deletes = { }
end

function PlayerComponent:AddPlayer(player)

    player:_Invoke("Awake")
    local player_id = player.player_id
    self.players[player_id] = player
end

function PlayerComponent:GetPlayer(player_id)
    return self.players[player_id]
end

function PlayerComponent:GetPlayers()
    return self.players
end

function PlayerComponent:DelPlayer(player_id)
    if self.players[player_id] ~= nil then
        table.insert(self.deletes, player_id)
    end
end

function PlayerComponent:OnUpdate()
    for _, player_id in ipairs(self.deletes) do
        self.players[player_id] = nil
    end
    self.deletes = { }
    for _, player in pairs(self.players) do
        player:_Invoke("OnUpdate")
    end
end

return PlayerComponent