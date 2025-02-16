local LuaListenComponent = {}

function LuaListenComponent:OnListen(sock)
	local str = sock:Read()
	print(str)
	sock:Close()
end


return LuaListenComponent