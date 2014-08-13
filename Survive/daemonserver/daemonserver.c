#include "kendynet.h"
#include "lua_util.h"
#include "stream_conn.h"
#include "common/netcmd.h"
#include "common/cmdhandler.h"
#include "common/common_c_function.h"

#define MAXCMD 65535
static cmd_handler_t handler[MAXCMD] = {NULL};
static engine_t t_engine = NULL;
static stream_conn_t to_router = NULL;    

static inline int call_lua_handler(luaTabRef_t *obj,uint16_t cmd,stream_conn_t conn,rpacket_t rpk){
		lua_State *L = obj->L;
		//get lua handle function
		lua_rawgeti(L,LUA_REGISTRYINDEX, obj->rindex);
		lua_pushinteger(L,cmd);
		lua_gettable(L,-2);
		lua_remove(L,-2);		
		//push arg
		if(rpk) 
			lua_pushlightuserdata(L,rpk);
		else 
			lua_pushnil(L);
		if(conn) 
			lua_pushlightuserdata(L,conn);
		else
			lua_pushnil(L);
		return lua_pcall(L,2,0,0);
}

static void process_cmd(uint16_t cmd,stream_conn_t conn,rpacket_t rpk){
	if(handler[cmd]){
		lua_State *L = handler[cmd]->obj->L;
		if(call_lua_handler(handler[cmd]->obj,cmd,conn,rpk)){
				const char *err = lua_tostring(L,1);
				lua_pop(L,1);
				printf("error on handle[%u]:%s\n",cmd,err);				
		}
	}else{
		printf("unknow cmd %d\n",cmd);
	}
}

static int on_packet(stream_conn_t conn,packet_t pk){
	rpacket_t rpk = (rpacket_t)pk;
	uint16_t cmd = rpk_read_uint16(rpk);
	process_cmd(cmd,conn,rpk);
	return 1;
}

static void on_disconnected(stream_conn_t conn,int err){
	//process_cmd(DUMMY_ON_GAME_DISCONNECTED,conn,NULL);
}

static void on_new_connection(handle_t s,void *_){
	stream_conn_t conn = new_stream_conn(s,65535,RPACKET);
	if(0 != stream_conn_associate(t_engine,conn,on_packet,on_disconnected))
		stream_conn_close(game);
}


static void cb_connect_router(handle_t s,int err,void *ud,kn_sockaddr *addr);
static int  cb_timer(kn_timer_t timer)//�������1����ע�ᣬ������ע��
{
	kn_sockaddr addr;
	kn_addr_init_in(&addr,"127.0.0.1",8888);
	handle_t sock = kn_new_sock(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	kn_sock_connect(t_engine,sock,&addr,NULL,cb_connect_router,NULL);
	return 0;
}


static void on_router_disconnected(stream_conn_t c,int err){
	to_router = NULL;
	kn_reg_timer(t_engine,5000,cb_timer,NULL);	
}

static void cb_connect_router(handle_t s,int err,void *ud,kn_sockaddr *addr)
{
	if(err == 0){
		//success
		to_router = new_stream_conn(s,65535,RPACKET);
		stream_conn_associate(t_engine,to_router,on_packet,on_router_disconnected);
		printf("connect to router success\n");
	}else{
		//failed
		kn_close_sock(s);
		kn_reg_timer(t_engine,5000,cb_timer,NULL);			
	}
}






/*

//to gateserver
static int on_gate_packet(stream_conn_t conn,packet_t pk){
	rpacket_t rpk = (rpacket_t)pk;
	uint16_t cmd = rpk_read_uint16(rpk);
	process_cmd(cmd,conn,rpk);
	return 1;
}

static void on_gate_disconnected(stream_conn_t conn,int err){
	process_cmd(DUMMY_ON_GATE_DISCONNECTED,NULL,NULL);
}


static void on_new_gate(handle_t s,void *_){
	stream_conn_t gate = new_stream_conn(s,65535,RPACKET);
	if(0 != stream_conn_associate(t_engine,gate,on_gate_packet,on_gate_disconnected))
		stream_conn_close(gate);
}

struct recon_ctx{
	handle_t     sock;
	kn_sockaddr  addr;
	void (*cb_connect)(handle_t,int,void*,kn_sockaddr*);
};

static int  cb_timer(kn_timer_t timer)//�������1����ע�ᣬ������ע��
{
	struct recon_ctx *recon = (struct recon_ctx*)kn_timer_getud(timer);
	kn_sock_connect(t_engine,recon->sock,&recon->addr,NULL,recon->cb_connect,NULL);
	free(recon);
	return 0;
}

//to chatserver
static stream_conn_t tochat = NULL;

static void cb_connect_chat(handle_t s,int err,void *ud,kn_sockaddr *addr);

static void on_chat_disconnected(stream_conn_t c,int err){
	tochat = NULL;
	struct recon_ctx *recon = calloc(1,sizeof(*recon));
	recon->sock = kn_new_sock(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	recon->cb_connect = cb_connect_chat;
	recon->addr = *kn_sock_addrpeer(stream_conn_gethandle(c));
	kn_reg_timer(t_engine,5000,cb_timer,recon);	
}

static int on_chat_packet(stream_conn_t conn,packet_t pk){
	((void)conn);
	((void)pk);
	return 1;
}

static void cb_connect_chat(handle_t s,int err,void *ud,kn_sockaddr *addr)
{
	if(err == 0){
		//success
		tochat = new_stream_conn(s,65535,RPACKET);
		stream_conn_associate(t_engine,tochat,on_chat_packet,on_chat_disconnected);
		printf("connect to chat success\n");					
		process_cmd(DUMMY_ON_CHAT_CONNECTED,tochat,NULL);
	}else{
		//failed
		kn_close_sock(s);
		struct recon_ctx *recon = calloc(1,sizeof(*recon));
		recon->sock = kn_new_sock(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		recon->addr = *addr;
		recon->cb_connect = cb_connect_chat;
		kn_reg_timer(t_engine,5000,cb_timer,recon);
	}
}

static int lua_send2chat(lua_State *L){
	wpacket_t wpk = lua_touserdata(L,1);
	if(0 == stream_conn_send(tochat,(packet_t)wpk))
		lua_pushboolean(L,1);
	else
		lua_pushboolean(L,0);
	return 1;	
}

//to mysql_proxy


static void sig_int(int sig){
	kn_stop_engine(t_engine);
}

int reg_cmd_handler(lua_State *L){
	uint16_t cmd = lua_tonumber(L,1);
	luaTabRef_t obj = create_luaTabRef(L,2);
	if(!handler[cmd]){
		printf("reg cmd %d\n",cmd);
		cmd_handler_t h = calloc(1,sizeof(*h));
		h->_type = FN_LUA;
		h->obj = calloc(1,sizeof(*h->obj));
		*h->obj = obj;
		handler[cmd] = h;
		lua_pushboolean(L,1);
	}else{
		release_luaTabRef(&obj);
		lua_pushboolean(L,0);
	}
	return 1;
}

static int lua_grouplog(lua_State *L){
	int lev = lua_tonumber(L,1);
	const char *msg = lua_tostring(L,2);
	LOG_GROUP(lev,"%s",msg);
	return 0;
}

void reg_group_c_function(lua_State *L){
	lua_getglobal(L,"GroupApp");
	if(!lua_istable(L, -1)){
		lua_pop(L,1);
		lua_newtable(L);
		lua_pushvalue(L,-1);
		lua_setglobal(L,"GroupApp");
	}
	
	REGISTER_FUNCTION("reg_cmd_handler",&reg_cmd_handler);
	REGISTER_FUNCTION("grouplog",&lua_grouplog);
	REGISTER_FUNCTION("send2chat",&lua_send2chat);
	lua_pop(L,1);
}

static lua_State *init(){
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	//ע��C������������lua
	reg_common_c_function(L);

	//ע��group���еĺ���
	reg_group_c_function(L);
	
	if (luaL_dofile(L,"script/handler.lua")) {
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		LOG_GROUP(LOG_INFO,"error on handler.lua:%s\n",error);
		printf("error on handler.lua:%s\n",error);
		lua_close(L); 
		return NULL;
	}

	//ע��lua��Ϣ������
	const char *error = NULL;
	if((error = LuaCall0(L,"reghandler",1))){
		LOG_GROUP(LOG_INFO,"error on reghandler:%s\n",error);
		printf("error on reghandler:%s\n",error);
		lua_close(L); 
	}
	
	int ret = lua_toboolean(L,1);
	lua_pop(L,1);
	if(!ret){
		LOG_GROUP(LOG_ERROR,"reghandler failed\n");
		printf("reghandler failed\n");
		return NULL;
	}
	return L;
}

int on_db_initfinish(lua_State *_){
	printf("on_db_initfinish\n");
	(void)_;
	//��������
	{
		//����gameserver������
		kn_sockaddr game_local;
		kn_addr_init_in(&game_local,kn_to_cstr(g_config->lgameip),g_config->lgameport);	
		handle_t l = kn_new_sock(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(0 != kn_sock_listen(t_engine,l,&game_local,on_new_game,NULL)){
			printf("create server on ip[%s],port[%u] error\n",kn_to_cstr(g_config->lgameip),g_config->lgameport);
			LOG_GROUP(LOG_INFO,"create server on ip[%s],port[%u] error\n",kn_to_cstr(g_config->lgameip),g_config->lgameport);	
			exit(0);
		}
	}
	
	{
		//����gateserver������		
		kn_sockaddr gate_local;
		kn_addr_init_in(&gate_local,kn_to_cstr(g_config->lgateip),g_config->lgateport);	
		handle_t l = kn_new_sock(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if(0 != kn_sock_listen(t_engine,l,&gate_local,on_new_gate,NULL)){
			printf("create server on ip[%s],port[%u] error\n",kn_to_cstr(g_config->lgateip),g_config->lgateport);
			LOG_GROUP(LOG_INFO,"create server on ip[%s],port[%u] error\n",kn_to_cstr(g_config->lgateip),g_config->lgateport);	
			exit(0);
		}
	}
	
	return 0;
} 

int main(int argc,char **argv){
	signal(SIGPIPE,SIG_IGN);	
	if(loadconfig() != 0){
		return 0;
	}
	signal(SIGINT,sig_int);
	t_engine = kn_new_engine();	
	if(!init())
		return 0;
	kn_engine_run(t_engine);
	return 0;	
}*/
