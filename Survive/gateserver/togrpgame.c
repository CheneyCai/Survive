#include "togrpgame.h"
#include "chanmsg.h"
#include "kn_stream_conn.h"
#include "config.h"

togrpgame*  g_togrpgame = NULL;

void forward_agent(rpacket_t rpk);

//������group��game����Ϣ
static int on_packet(kn_stream_conn_t con,rpacket_t rpk){
	forward_agent(rpk)	
	return 1;
}

static void on_connect_failed(kn_stream_client_t c,kn_sockaddr *addr,int err,void *ud)
{	
	if((remoteServerType)ud == GROUPSERVER){
		//��¼��־
	}else if((remoteServerType)ud == GAMESERVER){
		//��¼��־	
	}
	//����
	kn_stream_connect(c,NULL,addr,ud);
}

static void on_disconnected(kn_stream_conn_t conn,int err){
	if(conn == g_togrpgame->togroup){
		g_togrpgame->togroup = NULL;
	}else{
	
	}
}

static void on_connect(kn_stream_client_t c,kn_stream_conn_t conn,void *ud){
	if((remoteServerType)ud == GROUPSERVER){
		g_togrpgame->togroup = conn;
	}else if((remoteServerType)ud == GAMESERVER){
		
	}
	kn_stream_client_bind(g_togrpgame->stream_client,conn,0,65536,on_packet,on_disconnected,
			10*1000,NULL,0,NULL);
	
}

//��������channel����Ϣ
static void on_channel_msg(kn_channel_t chan, kn_channel_t from,void *msg,void *_)
{
	(void)_;
	if(((struct chanmsg*)msg)->msgtype == FORWARD_GAME){
		struct chanmsg_forward_game *_msg = (struct chanmsg_forward_game*)msg;
		kn_stream_conn_t conn = cast2_kn_stream_conn(_msg->game);
		if(conn){
			kn_stream_conn_send(conn,_msg->wpk);
			_msg->wpk = NULL;
		}
	}else if(((struct chanmsg*)msg)->msgtype == FORWARD_GROUP){
		struct chanmsg_forward_group *_msg = (struct chanmsg_forward_group*)msg;
		if(g_togrpgame->togroup){
			kn_stream_conn_send(g_togrpgame->togroup,_msg->wpk);
			_msg->wpk = NULL;
		}
	}
}

static void *service_main(void *ud){
	while(!g_togrpgame->stop){
		kn_proactor_run(g_togrpgame->p,50);
	}
	return NULL;
}

int     start_togrpgame(){
	g_togrpgame = calloc(1,sizeof(*g_togrpgame));
	g_togrpgame->p = kn_new_proactor();
	g_togrpgame->t = kn_create_thread(THREAD_JOINABLE);
	g_togrpgame->stream_client = kn_new_stream_client(g_togrpgame->p,
							  on_connect,
							  on_connect_failed);	

	g_togrpgame->chan = kn_new_channel(kn_thread_getid(g_togrpgame->t));
	kn_channel_bind(g_togrpgame->p,g_togrpgame->chan,on_channel_msg,NULL);
	kn_thread_start_run(g_togrpgame->t,service_main,NULL);
	return 0;


}

void    stop_togrpgame(){
	g_togrpgame->stop = 1;
	kn_thread_join(g_togrpgame->t);
}

