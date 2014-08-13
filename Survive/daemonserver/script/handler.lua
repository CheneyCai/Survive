local Gate = require "script/gate"
local Game = require "script/game"
local Dbmgr = require "script/dbmgr"
local Player = require "script/player"
local Rpc = require "script/rpc"
local Cjson = require "cjson"

local forbidwords = {}
table.insert(forbidwords,"������")

--ע���ģ�����Ϣ������
function reghandler(dbconfig)
	dbconfig = Cjson.decode(dbconfig)
	Rpc.RegHandler()
	return Dbmgr.Init(dbconfig)
end


