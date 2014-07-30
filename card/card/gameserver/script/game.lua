local Que = require "script/queue"
local Avatar = require "script/avatar"
local Cjson = require "cjson"
local Dbmgr = require "script/dbmgr"

local map101 = require "script/map101"
local map102 = require "script/map102"
local map103 = require "script/map103"

local function load_login_callback(self,error,result)
	print("load_login_callback")
	if error then
		print("error1")
		self.ply:on_error()
		return
	end
	
	if not result then
		print("error2")
		self.ply:on_error()
		return 
	end
	
	local key = result[1]
	local role = result[2]
	print("-----------------")
	print(self.key)
	print(key)
	if self.key ~= key then
		print("error3")
		self.ply:on_error()
		return 
	end
	
	if not role then
		--֪ͨ������ɫ
		print("notify_create")
		self.ply:notify_create()
		return 
	end	
	self.ply.role = Cjson.decode(role)
	self.ply:notify_login_success()
	self.ply:NotifyRoleInfo()
	self.ply.status = stat_playing		
end


local function CMD_LOGIN(_,rpk,conn)
	local usrid = rpk_read_uint32(rpk);
	local key = rpk_read_string(rpk);
	print("CMD_LOGIN")
	print(usrid)
	print(key)
	if Avatar.GetPlyByConn(conn) then
		return
	end
	local player = Avatar.GetPlyById(usrid)	
	if player then
		C.close(conn)
	end
	--��redis�����û�����	
	local player = Avatar.NewPlayer(conn,usrid)
	local cmd = "hmget usrid:" .. usrid .. " key" .. " role" .. " card" .. " map" .. " mapstory"
	local err = Dbmgr.DBCmd(usrid,cmd,{callback = load_login_callback,ply=player,key=key})
	if err then
		C.close(conn)
		return
	end
	player.status = stat_loading	
end

local function CREATE_ROLE(_,rpk,conn)
	print("CREATE_ROLE")
	local player = Avatar.GetPlyByConn(conn)
	if not player then
		C.close(conn)
		return
	end
	local rolename = rpk_read_string(rpk)
	player:CreateRole(rolename)
end


local function CLIENT_DISCONN(_,rpk,conn)
	print("CLIENT_DISCONN")
	Avatar.DestryPlayer(Avatar.GetPlyByConn(conn))
end

local function SendMapReward(conn)

end

local function SendMapInfo(conn,mapinfo)	
	local wpk = new_wpk(4096)
	wpk_write_uint16(wpk,CSID_MAP_INFO)
	wpk_write_uint16(wpk,mapinfo.MapId)
	wpk_write_uint16(wpk,mapinfo.MapStar)
	wpk_write_uint16(wpk,mapinfo.RolePos)
	wpk_write_uint32(wpk,mapinfo.TotalGold)
	wpk_write_uint32(wpk,mapinfo.TotalTreaBox)
	wpk_write_uint16(wpk,mapinfo.StoryId)
	wpk_write_uint8(wpk,mapinfo.byCount)
	for i=1,80 do
		wpk_write_uint8(wpk,mapinfo.MapInfo[i].PointMoved)
		wpk_write_uint8(wpk,mapinfo.MapInfo[i].PointBlocked)
		wpk_write_uint16(wpk,mapinfo.MapInfo[i].ObjectId)
		wpk_write_uint16(wpk,mapinfo.MapInfo[i].TileId)		
	end
	C.send(conn,wpk)
end

local function ENTERMAP_REQ(_,rpk,conn)
	print("ENTERMAP_REQ")
	local mapid = rpk_read_uint32(rpk)
	local diffculty = rpk_read_uint32(rpk)
	
	local ply = Avatar.GetPlyByConn(conn)
	if not ply then
		C.close(conn)
		return
	end
	
	SendMapReward(conn)
	
	if mapid == 101 then
		ply.map = map101
	elseif mapid == 102 then
		ply.map = map102
	else
		ply.map = map103
	end
	SendMapInfo(conn,ply.map.map)
	
	local wpk = new_wpk(64)
	wpk_write_uint16(wpk,CSID_ENTERMAP_ACK)
	wpk_write_uint16(wpk,0)
	C.send(conn,wpk)
	
end


local function MOVETEST_REQ(_,rpk,conn)
	local ply = Avatar.GetPlyByConn(conn)
	if not ply then
		C.close(conn)
		return
	end
	
	local mapidx = rpk_read_uint32(rpk)
	local wpk = new_wpk(64)
	wpk_write_uint16(wpk,CSID_MOVETEST_ACK)
	wpk_write_uint16(wpk,0)
	C.send(conn,wpk)	
	if mapidx == 19 then
		wpk = new_wpk(64)
		wpk_write_uint16(wpk,CSID_MAPPOINT_INFO)
		wpk_write_uint16(wpk,8)
		C.send(conn,wpk)
	end	
end

local function PREFIGHT_REQ(_,rpk,conn)
	print("PREFIGHE_REQ")
	local ply = Avatar.GetPlyByConn(conn)
	if not ply then
		C.close(conn)
		return
	end
	
	local wpk = new_wpk(64)
	wpk_write_uint16(wpk,CSID_PREFIGHT_INFO)
	local fightinfo = ply.map.fightinfo
	wpk_write_uint16(wpk,fightinfo.StoryId)
	wpk_write_uint8(wpk,#fightinfo.astTeam1)
	for i = 1,#fightinfo.astTeam1 do
		wpk_write_uint32(wpk,fightinfo.astTeam1[i].CardUId)
		wpk_write_uint16(wpk,fightinfo.astTeam1[i].CardId)
		wpk_write_uint16(wpk,fightinfo.astTeam1[i].CardLvl)		
		wpk_write_uint16(wpk,fightinfo.astTeam1[i].CurrentHP)
		wpk_write_uint8(wpk,fightinfo.astTeam1[i].byLive)
		wpk_write_uint16(wpk,fightinfo.astTeam1[i].MaxHP)			
	end
	wpk_write_uint8(wpk,#fightinfo.astTeam2)
	for i = 1,#fightinfo.astTeam2 do
		wpk_write_uint32(wpk,fightinfo.astTeam2[i].CardUId)
		wpk_write_uint16(wpk,fightinfo.astTeam2[i].CardId)
		wpk_write_uint16(wpk,fightinfo.astTeam2[i].CardLvl)		
		wpk_write_uint16(wpk,fightinfo.astTeam2[i].CurrentHP)
		wpk_write_uint8(wpk,fightinfo.astTeam2[i].byLive)
		wpk_write_uint16(wpk,fightinfo.astTeam2[i].MaxHP)			
	end		
	C.send(conn,wpk)
	wpk = new_wpk(64)
	wpk_write_uint16(wpk,CSID_PREFIGHT_ACK)
	wpk_write_uint16(wpk,0)
	C.send(conn,wpk)
	print("here1")			
end

local function ENTERFIGHT_REQ(_,rpk,conn)
	print("ENTERFIGHT_REQ")
	local ply = Avatar.GetPlyByConn(conn)
	if not ply then
		C.close(conn)
		return
	end
	local wpk = new_wpk(64)
	wpk_write_uint16(wpk,CSID_FIGHT_FRAME_DATA)
	local fightinfo = ply.map.fightframe
	wpk_write_uint8(wpk,fightinfo.FightInfo.FightEnd)
	wpk_write_uint8(wpk,#fightinfo.FightInfo.astTeam1)
	for i = 1,#fightinfo.FightInfo.astTeam1 do
		wpk_write_uint32(wpk,fightinfo.FightInfo.astTeam1[i].CardUId)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam1[i].CardId)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam1[i].CardLvl)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam1[i].CurrentHP)
		wpk_write_uint8(wpk,fightinfo.FightInfo.astTeam1[i].byLive)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam1[i].MaxHP)
	end	
	wpk_write_uint8(wpk,#fightinfo.FightInfo.astTeam2)
	for i = 1,#fightinfo.FightInfo.astTeam2 do
		wpk_write_uint32(wpk,fightinfo.FightInfo.astTeam2[i].CardUId)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam2[i].CardId)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam2[i].CardLvl)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam2[i].CurrentHP)
		wpk_write_uint8(wpk,fightinfo.FightInfo.astTeam2[i].byLive)
		wpk_write_uint16(wpk,fightinfo.FightInfo.astTeam2[i].MaxHP)
	end
	
	wpk_write_uint16(wpk,#fightinfo.astFrameInfo);
	for i = 1,#fightinfo.astFrameInfo do
		wpk_write_uint8(wpk,fightinfo.astFrameInfo[i].type)
		if fightinfo.astFrameInfo[i].type == 0 then
			wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].PlayerCardUId)
			wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].PlayerCardId)
			wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].RobotCardUId)
			wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].RobotCardId)
		elseif fightinfo.astFrameInfo[i].type == 1 then
			wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].SkillId)
			wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].FromRace)
			wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].FromCardUId)
			wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].FromCardId)
			wpk_write_uint8(wpk,#fightinfo.astFrameInfo[i].astTargetCards)
			for j = 1,#fightinfo.astFrameInfo[i].astTargetCards do
				wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].Race)
				wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].CardUId)
				wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].CardId)
				wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].Value)
				wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].CurrentHP)
				wpk_write_uint8(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].byLive)
				wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].astTargetCards[j].MaxHP)
			end			
		else
			wpk_write_uint16(wpk,fightinfo.astFrameInfo[i].SkillId)
			wpk_write_uint8(wpk,#fightinfo.astFrameInfo[i].astSkillEffs)
			for j = 1,#fightinfo.astFrameInfo[i].astSkillEffs do
				wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].astSkillEffs[j].EffType)
				wpk_write_uint32(wpk,fightinfo.astFrameInfo[i].astSkillEffs[j].EffValue)
			end
		end
	end	
	C.send(conn,wpk)
	
	wpk = new_wpk(64)
	wpk_write_uint16(wpk,CSID_ENTERFIGHT_ACK)
	wpk_write_uint16(wpk,0)
	C.send(conn,wpk)
end	



local function reg_cmd_handler()
	C.reg_cmd_handler(CSID_MOVETEST_REQ,{handle=MOVETEST_REQ})
	C.reg_cmd_handler(CSID_LOGIN_REQ,{handle=CMD_LOGIN})
	C.reg_cmd_handler(CSID_CREATE_ROLE_REQ,{handle=CREATE_ROLE})
	C.reg_cmd_handler(DUMMY_ON_CLI_DISCONNECTED,{handle=CLIENT_DISCONN})
	C.reg_cmd_handler(CSID_ENTERMAP_REQ,{handle=ENTERMAP_REQ})
	C.reg_cmd_handler(CSID_PREFIGHT_REQ,{handle=PREFIGHT_REQ})
	C.reg_cmd_handler(CSID_ENTERFIGHT_REQ,{handle=ENTERFIGHT_REQ})			
end

return {
	RegHandler = reg_cmd_handler,
}


