#ifndef _CONFIG_H
#define _CONFIG_H

#include "kn_string.h"

typedef struct config{
	//��gateserver���ӵļ���
	kn_string_t lgateip;
	uint16_t    lgateport;
}config;

extern config* g_config;

int loadconfig(); 








#endif
