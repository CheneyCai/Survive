local Gate = require "script/gate"
local Game = require "script/game"
local Dbmgr = require "script/dbmgr"



local forbidwords = {}
table.insert(forbidwords,"������")

--ע���ģ�����Ϣ������
function reghandler()
	Gate.RegHandler()
	Game.RegHandler()
	initwordfilter(forbidwords)
	return Dbmgr.Init()
end

