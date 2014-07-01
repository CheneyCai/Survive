local Avatar = require "script/avatar"
local Que = require "script/queue"

local astarmgr = {}

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

local function map:new(o)
  o = o or {}   
  setmetatable(o, self)
  self.__index = self
  return o
end

local function map:init(mapid,maptype)
	self.mapid = mapid
	self.maptype = maptype
	self.freeidx = Que.Queue()
	for i=1,65535 do
		self.freeidx:push({v=i,__next=nil})
	end
	self.astar = astarmgr[maptype]
	--�����߳�,��׼�Ӿ�,���Ͻ�x,���Ͻ�y,���½�x,���½�y	
	self.aoi = GameApp.create_aoimap(100,100,0,0,100,100)
	local m = self
	--ע�ᶨʱ��
	self.movtimer = C.reg_timer(100,{on_timeout = function (_)
										m:process_mov()
										return 1				
									 end})
	return self
end

local function read_player_from_rpk(rpk)
	
end

local function map:entermap(rpk)
	local plys = read_player_from_rpk(rpk)
	if self.freeidx:len() < #plys then
		--û���㹻��id�������avatar
		return false
	else
		for _,v in pairs(plys) do
			--TODO ������Ϣ����avatar
		end
		
		--ͨ��group�����ͼ�������
		
		return true
	end
end

local function map:leavemap(plyid)
	local ply = self.avatars[plyid]
	if ply and ply.avattype == Avatar.type_player then
		--�����뿪��ͼ
	end
end

local function map:findpath(from,to)
	return GameApp.findpath(self.astar,from[1],from[2],to[1],to[2])
end

--��avatar��ӵ��ƶ������б���
local function map:beginMov(avatar)
	if not self.movingavatar[avatar.id] then
		self.movingavatar[avatar.id] = avatar
	end
end

--��ͼ����֮ǰ���������
local function map:clear()
	GameApp.destroy_aoimap(self.aoi)
	C.del_timer(self.movtimer)
end

--������ͼ�ϵĶ����ƶ�����
local function map:process_mov()
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
