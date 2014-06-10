#ifndef _TOGRPGAME_H
#define _TOGRPGAME_H

#include "kn_stream_conn_client.h"
#include "kn_thread.h"
#include "kendynet.h"

//��group��game�����Ӵ������,�����ڶ������߳���

typedef struct togrpgame{
	kn_proactor_t       p;
	kn_thread_t         t;
	kn_channel_t        chan;
	volatile uint8_t    stop;
	kn_stream_conn_t    togroup;
	kn_stream_client_t  stream_client;
}togrpgame;


int     start_togrpgame();
void    stop_togrpgame();

extern togrpgame * g_togrpgame;

#endif
