local Avatar = require "script/avatar"
local Que = require "script/queue"
local MapConfig = require "script/mapconfig"
local Cjson = require "cjson"
local Gate = require "script/gate"

local map = {
	maptype,
	mapid,
	astar,
	aoi,
	avatars,
	freeidx,
	plycount,      --��ͼ����ҵ�����
	movingavatar,  --���ƶ������avatar
	movtimer,      --�ƶ�����ʱ��
}

function map:new(o)
  o = o or {}   
  setmetatable(o, self)
  self.__index = self
  return o
end

function map:init(mapid,maptype)
	self.mapid = mapid
	self.maptype = maptype
	self.freeidx = Que.Queue()
	for i=1,65535 do
		self.freeidx:push({v=i,__next=nil})
	end
	
	local mapdef = MapConfig.GetDefByType(maptype)	
	self.astar = mapdef.astar
	--�����߳�,��׼�Ӿ�,���Ͻ�x,���Ͻ�y,���½�x,���½�y	
	self.aoi = GameApp.create_aoimap(mapdef.gridlength,
			   mapdef.radius,mapdef.toleft[1],mapdef.toleft[2],mapdef.bottomright[1],mapdef.bottomright[2])
	local m = self
	--ע�ᶨʱ��
	self.movtimer = C.reg_timer(100,{on_timeout = function (_)
										m:process_mov()
										return 1				
									 end})
	return self
end


function map:entermap(rpk)
	local plys = Cjson.decode(rpk_read_string(rpk))
	if self.freeidx:len() < #plys then
		--û���㹻��id�������avatar
		return nil
	else
		local gameids = {}
		for _,v in pairs(plys) do
			--TODO ������Ϣ����avatar
			local avatid = v.avatid
			local gate = Gate.GetGateByName(v.gate.name)
			if not gate then
				return nil
			end
			local gateid = v.gate.id
			local ply = Avatar.NewPlayer(self.freeidx:pop(),avatid)
			ply.gate = {conn=gate.conn,id=gateid}
			ply.nickname = v.nickname
			ply.groupid = v.groupid
			--�����ʵid��16λ��ͼid,��16λ���id
			ply.id = self.mapid * 65536 + ply.id
			table.insert(gameids,ply.id)
			print(v.nickname .. " enter map")
		end
		
		--TODO aoi����ͼ
		
		--TODO ͨ��group�����ͼ������� 
		return gameids
	end
end

function map:leavemap(plyid)
	local ply = self.avatars[plyid]
	if ply and ply.avattype == Avatar.type_player then
		--�����뿪��ͼ
		--TODO aoi�뿪��ͼ
		return true
	end
	return false
end

function map:findpath(from,to)
	return GameApp.findpath(self.astar,from[1],from[2],to[1],to[2])
end

--��avatar��ӵ��ƶ������б���
function map:beginMov(avatar)
	if not self.movingavatar[avatar.id] then
		self.movingavatar[avatar.id] = avatar
	end
end

--��ͼ����֮ǰ���������
function map:clear()
	GameApp.destroy_aoimap(self.aoi)
	C.del_timer(self.movtimer)
end

--������ͼ�ϵĶ����ƶ�����
function map:process_mov()
	local stops = {}
	for k,v in pairs(self.movingavatar) do
		if v:process_mov() then
			table.insert(stops,k)
		end
	end
	
	for k,v in pairs(stops) do
		self.movingavatar[v] = nil
	end
end 

return {
	NewMap = function (mapid,maptype) return map:new():init(mapid,maptype) end,
}
