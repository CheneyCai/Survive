#include "agent.h"

/*
*  �˺��ķ�����
*  1���߳��ܼ���
*  1���߳���togrpgame
*  6���߳���agent
*/

#define MAX_AGENT 8
static agent *agents[MAX_AGENT];

static void on_new_client(kn_stream_server_t _,kn_stream_conn_t conn){
	(void)_;
	//���ѡ��һ��agent��conn����������
}

static int agent_count;
static volatile int stop = 0;
static void sig_int(int sig){
	stop = 1;
}

int main(int argc,char **argv){

	signal(SIGINT,sig_int);
	int i = 0;
	agent_count = 6;//ͨ�����û�����������ȡ
	for(; i < agent_count; ++i)
		agents[i] = start_agent(i);

	kn_proactor_t p = kn_new_proactor();
	//��������
	kn_sockaddr local;
	kn_addr_init_in(&local,argv[1],atoi(argv[2]));
	kn_new_stream_server(p,&local,on_new_client);
	
	while(!stop)
		kn_proactor_run(p,50);
		
	//for(i=0;i < MAX_AGENT; ++i)
	//	stop_agent(agents[i]);

	return 0;
}