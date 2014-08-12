#ifndef _TOINNER_H
#define _TOINNER_H

#include "kn_thread.h"
#include "kn_thread_mailbox.h"
#include "kendynet.h"
#include "stream_conn.h"

//��group,game,chat���ڲ������������Ӵ������,�����ڶ������߳���

typedef struct toinner{
	engine_t            p;
	kn_thread_t         t;
	kn_thread_mailbox_t mailbox;
	stream_conn_t       togroup;
	stream_conn_t       tochat;
}toinner;


int     start_toinner();
void    stop_toinner();
int     mail2inner(void*,void (*fn_destroy)(void*));

extern toinner * g_toinner;

#endif
