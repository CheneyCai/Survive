local Gate = require "gate"
local Game = require "game"
local Dbmgr = require "dbmgr"

--ע���ģ�����Ϣ������
function reghandler()
	Gate.RegHandler()
	Game.RegHandler()
	return Dbmgr.Init()
end

