local Gate = require "script/gate"
local Game = require "script/game"
local Avatar = require "script/avatar"
local Dbmgr = require "script/dbmgr"
local Rpc = require "script/rpc"
local Cjson = require "cjson"

--ע���ģ�����Ϣ������
function reghandler(dbconfig)
	dbconfig = Cjson.decode(dbconfig)
	Gate.RegHandler()
	Game.RegHandler()
	Rpc.RegHandler()

	--[[local mapdef = MapConfig.GetDefByType(1)
	GameApp.create_aoimap(mapdef.gridlength,
			   mapdef.radius,mapdef.toleft[1],mapdef.toleft[2],mapdef.bottomright[1],mapdef.bottomright[2])	
--]]	
	
	return Dbmgr.Init(dbconfig)	
end
