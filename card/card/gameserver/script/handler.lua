local Game = require "script/game"
local Avatar = require "script/avatar"
local Dbmgr = require "script/dbmgr"

--ע���ģ�����Ϣ������
function reghandler()
	Game.RegHandler()
	return Dbmgr.Init()	
end
