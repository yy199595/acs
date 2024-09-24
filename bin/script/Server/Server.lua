local proto = require("Proto")
local Module = require("Module")
local http = require("HttpComponent")
local mongo = require("MongoComponent")
local redis = require("RedisComponent")

local Main = Module()

SetMember(Main, "count", 1)

proto.Import("record/record.proto")

function Main:Awake()
	
	table.print(Socket.Query("www.baidu.com"))

	local head = { }
	table.insert(head, "GET / HTTP/1.0")
	table.insert(head, "Content-Length: 0")
	table.insert(head, "Connection: close")
	table.insert(head, "\r\n")
	local data = table.concat(head, "\r\n")
	local sock = Socket.Connect("www.baidu.com", 80)

	sock:Send(data)
	print(sock:Read())
end

function Main:OnComplete()

end


return Main
