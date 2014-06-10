#ifndef _NETCMD_H
#define _NETCMD_H


enum{
	//client <-> agent
	CMD_CA_BEGIN = 0,
	CMD_CA_END,

	CMD_AC_BEGIN = CMD_CA_BEGIN + 1,
	CMD_AC_END,

	//client <-> game
	CMD_CS_BEGIN = CMD_AC_END + 1,
	CMD_CS_END,

	CMD_SC_BEGIN = CMD_CS_END + 1,
	CMD_SC_END,

	//client <-> group
	CMD_CG_BEGIN = CMD_SC_END + 1,
	CMD_CG_END,

	CMD_GC_BEGIN = CMD_CG_END + 1,
	CMD_GC_END,

	//gate <-> group
	CMD_AG_BEGIN = CMD_GC_END + 1,
	CMD_AG_END,

	CMD_GA_BEGIN = CMD_AG_END + 1,
	CMD_GA_END,

	//game <-> group

	CMD_GAMEG_BEGIN = CMD_GA_END + 1,
	CMD_GAMEG_END,

	CMD_GGAME_BEGIB = CMD_GAMEG_END + 1,
	CMD_GGAME_END,

};



/*enum{
	//�ͻ��˵������
	CMD_CS_BEGPLY = 1,//�����������ͼ����,
	CMD_CS_MOV,       //�����ƶ�

	//����˵��ͻ���
	CMD_SC_BEGPLY,    //֪ͨ��ҽ����ͼ�����ɹ������Դ�����ͼ������
	CMD_SC_ENTERSEE,  //�������������Ұ
	CMD_SC_LEVSEE,    //�����뿪������Ұ
	CMD_SC_ENDPLY,    //�����뿪������ͼ
	CMD_SC_MOV,       //�����ƶ�

};*/





#endif