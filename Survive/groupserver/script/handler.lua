local Gate = require "gate"
local Game = require "game"
local Dbmgr = require "dbmgr"



local forbidwords = {}
table.insert(forbidwords,"������")

--ע���ģ�����Ϣ������
function reghandler()
	Gate.RegHandler()
	Game.RegHandler()
	initwordfilter(forbidwords)
	return Dbmgr.Init()
end

