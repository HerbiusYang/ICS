#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	char expr[32]; //expression
	
	unit32_t result;  //the orthx of expression



	/* TODO: Add more members if necessary */


} WP;


WP *head;

static WP* new_wp();

void free_wp(WP *wp);  // two function of PA1.3


#endif
