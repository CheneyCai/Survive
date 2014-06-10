#ifndef _GATEPLAYER_H
#define _GATEPLAYER_H

#include "kn_string.h"
#include "kn_ref.h"
#include "common/agentsession.h"

typedef enum{
	ply_init = 1,             //���ӽ������ȴ���������û�������  
	ply_wait_verify,          //����Ѿ������û���������ȴ���֤���
	ply_wait_group_confirm,   //��֤��ϵȴ�groupȷ��
	ply_create,               //������ɫ
	ply_playing,              //���������Ϸ�� 
}plystate;


typedef struct agentplayer{
	kn_ref           ref;
	agentsession     agentsession;
	kn_stream_conn_t toclient;
	ident            togame;
	uint32_t         gameid;
	uint32_t         groupid;
	kn_string_t      actname;
	plystate         state;
}agentplayer,*agentplayer_t;

#endif