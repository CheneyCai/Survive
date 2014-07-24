#include "kendynet.h"
#include "gameserver.h"
#include "config.h"
#include "lua/lua_util.h"
#include "kn_stream_conn_server.h"
#include "common/netcmd.h"
#include "common/cmdhandler.h"
#include "common/common_c_function.h"

IMP_LOG(gamelog);

#define MAXCMD 65536
static cmd_handler_t handler[MAXCMD] = {NULL};

__thread kn_proactor_t t_proactor = NULL;

static int on_packet(kn_stream_conn_t conn,rpacket_t rpk){
	uint16_t cmd = rpk_read_uint16(rpk);	
	printf("gate_packet:%u\n",cmd);
	if(handler[cmd]){
		lua_State *L = handler[cmd]->obj->L;
		const char *error = NULL;
		if((error = CALL_OBJ_FUNC2(handler[cmd]->obj,"handle",0,
						  lua_pushlightuserdata(L,rpk),
						  lua_pushlightuserdata(L,conn)))){
			LOG_GAME(LOG_INFO,"error on handle[%u]:%s\n",cmd,error);
			printf("error on handle[%u]:%s\n",cmd,error);
		}
	}
	return 1;
}

static void on_disconnected(kn_stream_conn_t conn,int err){
	uint16_t cmd = DUMMY_ON_CLI_DISCONNECTED;
	if(handler[cmd]){
		lua_State *L = handler[cmd]->obj->L;
		const char *error = NULL;
		if((error = CALL_OBJ_FUNC2(handler[cmd]->obj,"handle",0,
						  lua_pushnil(L),lua_pushlightuserdata(L,conn)))){
			LOG_GAME(LOG_INFO,"error on handle[%u]:%s\n",cmd,error);
		}
	}	
}

static void on_new_cli(kn_stream_server_t server,kn_stream_conn_t conn){
	printf("on_new_cli\n");
	if(0 == kn_stream_server_bind(server,conn,0,65536,
				      on_packet,on_disconnected,
				      0,NULL,0,NULL)){
	}else{
		kn_stream_conn_close(conn);
	}
}

int reg_cmd_handler(lua_State *L){
	uint16_t cmd = lua_tonumber(L,1);
	luaObject_t obj = create_luaObj(L,2);
	if(!handler[cmd]){
		printf("reg cmd %d\n",cmd);
		cmd_handler_t h = calloc(1,sizeof(*h));
		h->_type = FN_LUA;
		h->obj = obj;
		handler[cmd] = h;
		lua_pushboolean(L,1);
	}else{
		release_luaObj(obj);
		lua_pushboolean(L,0);
	}
	return 1;
}


static int lua_gamelog(lua_State *L){
	int lev = lua_tonumber(L,1);
	const char *msg = lua_tostring(L,2);
	LOG_GAME(lev,"%s",msg);
	return 0;
}

void reg_game_c_function(lua_State *L){
	lua_getglobal(L,"GameApp");
	if(!lua_istable(L, -1))
	{
		lua_pop(L,1);
		lua_newtable(L);
		lua_pushvalue(L,-1);
		lua_setglobal(L,"GameApp");
	}
	
	REGISTER_FUNCTION("gamelog",&lua_gamelog);	

	lua_pop(L,1);
}

static lua_State *init(){
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	//ע��C������������lua
	reg_common_c_function(L);

	//ע��group���еĺ���
	reg_game_c_function(L);

	if (luaL_dofile(L,"script/handler.lua")) {
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		LOG_GAME(LOG_INFO,"error on handler.lua:%s\n",error);
		printf("error on handler.lua:%s\n",error);
		lua_close(L); 
		return NULL;
	}

	//ע��lua��Ϣ������
	if(CALL_LUA_FUNC(L,"reghandler",0)){
		const char * error = lua_tostring(L, -1);
		lua_pop(L,1);
		LOG_GAME(LOG_INFO,"error on reghandler:%s\n",error);
		printf("error on handler.lua:%s\n",error);
		lua_close(L); 
	}
	return L;
}

static volatile int stop = 0;
static void sig_int(int sig){
	stop = 1;
}

int on_db_initfinish(lua_State *_){
	(void)_;
	printf("on_db_initfinish\n");
	//��������
	kn_sockaddr lgateserver;
	kn_addr_init_in(&lgateserver,kn_to_cstr(g_config->lgateip),g_config->lgateport);
	kn_new_stream_server(t_proactor,&lgateserver,on_new_cli);

	return 0;
} 

int main(int argc,char **argv){
	signal(SIGPIPE,SIG_IGN);	
	signal(SIGINT,sig_int);
	t_proactor = kn_new_proactor();
	if(loadconfig() != 0){
		return 0;
	}

	printf("%d\n",stop);
	if(!init())
		return 0;

	while(!stop){
		kn_proactor_run(t_proactor,100);
	}

	return 0;	
}



