local Gate = require "gate"
local Game = require "game"
local Avatar = require "avatar"

--ע���ģ�����Ϣ������
function reghandler()
	Gate.RegHandler()
	Game.RegHandler()
	AVatar.RegHandler()
end