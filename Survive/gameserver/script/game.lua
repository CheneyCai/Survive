local Map = require "script/map"
local Que = require "script/queue"
local Avatar = require "script/avatar"

local game = {
	id,
	maps,
	freeidx,
}

function game_init(id)
	game.id = id
	game.maps = {}
	local que = Que.Queue()
	for i=1,65535 do
		que:push({v=i,__next=nil})
	end
	game.freeidx = que
end


local function GGAME_ENTERMAP(_,rpk,conn)
	local mapid = rpk_read_uint16(rpk)
	local maptype = rpk_read_uint8(rpk)
	if not mapid then
		--����ʵ��
		mapid = game.freeidx:pop()
		if not mapid then
			--TODO ֪ͨgroup,gameserver��æ
		else
			local map = Map.NewMap():init(mapid,maptype)
			game.maps[mapid] = map
			if not map:entermap(rpk) then
				--֪ͨgroup�����ͼʧ��
			end
		end
	else
		local map = game.maps[mapid]
		if not map then
			--TODO ֪ͨgroup�����mapid(����ʵ���Ѿ�������)
		else
			if not map:entermap(rpk) then
				--֪ͨgroup�����ͼʧ��
			end
		end
	end
end

local function CGAME_LEAVEMAP(_,rpk,conn)
	local mapid = rpk_read_uint16(rpk)
	local map = game.maps[mapid]
	if map then
		local plyid = rpk_read_uint16(rpk)
		map:leavemap(plyid)
	end
end


local function GGAME_DESTROYMAP(_,rpk,conn)
	local mapid = rpk_read_uint16(rpk)
	local map = game.maps[mapid]
	if map then
		map:clear()
		game.que:push({v=mapid,__next=nil})
		game.maps[mapid] = nil
	end
end


local function CS_MOV(_,rpk,conn)
	local mapid = rpk_read_uint16(rpk)
	local map = game.maps[mapid]
	if map then
		local plyid = rpk_read_uint16(rpk)
		local ply = map.avatars[plyid]
		if ply and ply.avattype == Avatar.type_player then
			local x = rpk_read_uint16(rpk)
			local y = rpk_read_uint16(rpk)
			ply:mov(x,y)
		end
	end
end


local function reg_cmd_handler()
	GameApp.reg_cmd_handler(CMD_GGAME_ENTERMAP,{handle=GGAME_ENTERMAP})
	GameApp.reg_cmd_handler(CMD_CGAME_LEAVEMAP,{handle=CGAME_LEAVEMAP})
	GameApp.reg_cmd_handler(CMD_GGAME_DESTROYMAP,{handle=GGAME_DESTROYMAP})
	GameApp.reg_cmd_handler(CMD_CS_MOV,{handle=CS_MOV})	
end

return {
	RegHandler = reg_cmd_handler,
}


