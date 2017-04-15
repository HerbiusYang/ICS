#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_list[NR_WP];
static WP *free_;

void init_wp_list() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_list[i].NO = i;
		wp_list[i].next = &wp_list[i + 1];
	}
	wp_list[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_list;
}

/* TODO: Implement the functionality of watchpoint */

static WP* new_wp(){
	WP* temp;

	if (free_ == NULL){
		Assert(0."");
	}

	temp = free_;

	free_ = free_ -> next;
	WP* status_H = head;

	if (head == NULL){
		head = temp;
		temp->next = NULL;
	}
	else {
			while(status_H->next!=NULL){
			status_H = status_H->next;
	}
	status_H->next = temp;
	temp->next = NULL;
	}
	return temp;
}

void free_wp(WP *wp){

	assert(wp!=NULL,"Don't have this watvhpoint...");
	WP* status_H = head;
	if (status_H == wp)
	{
		head = head->next;
	}
	else
	{
		while(status_H!=NULL){
		if (wp == status_H->next) {
		status_H->next = status_H->next->next;
		}
		status_H = status_H->next;

		}
	}

	WP* status_F = free_;
	if (free_==NULL) {
		free_ = wp;
	}
	else{
		while (status_F->next!=NULL){
			status_F = status_F->next;
		}
	status_F->next = wp;
	wp->next = NULL;
	}
}


