#ifndef _GATEPLAYER_H
#define _GATEPLAYER_H

#include "kn_string.h"
#include "kn_ref.h"
#include "common/agentsession.h"

enum
{
	agent_unusing = 0,  //û������
	agent_init,
	agent_verifying,    //�ȴ��˺���֤
	agent_playing,      //������Ϸ
	agent_creating,     //���ڴ����˺���Ϣ
};

typedef struct battlesession{
	ident     tomap;     //����ͼ������������
	uint16_t  mapid;     //��ͼʾ��id
	uint16_t  objid;     //��ͼ�ж���id
}battlesession;

//gateserver�е��û���ʾ�ṹ
typedef struct agentplayer{
	agentsession    session;
	uint16_t        identity;
	ident           toclient;      //���ͻ��˵�����
	battlesession*  battlesession;
	uint16_t        groupid;       //�����groupserver�ϵĶ���id
	uint8_t         state;
	kn_string_t     actname;       //�ʺ��� 
}agentplayer,*agentplayer_t;

#endif