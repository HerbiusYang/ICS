#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);


/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

// Add Command Function part 

static void dump_register() {
    int i;
    for(i = R_EAX; i <= R_EDI; i ++) {
        printf("%s: 0x%08x %08d\n", regsl[i], cpu.gpr[i]._32 , cpu.gpr[i]._32);
    }
    printf("eip: 0x%08x %08d\n", cpu.eip, cpu.eip);

}

static int cmd_info(char *args) {
    switch (*args) {
    case 'r':dump_register(); return 0;
    default: return 1;
    }
}

static int cmd_x(char *args) {

    unsigned int addr, len, i;
    sscanf(args, "%d 0x%x", &len, &addr);   //invoke sscnaf() function in Internet  instead of strvok()
    printf("dump memory start addr: 0x%08x len: %d\n", addr, len);
    for (i = 0; i < len; ++i) {
        if (!(i && (i%5!=0)))                        // Why? Interesting... of symbol &
        	printf("\n0x%08x: ", addr + i);        //Please mention the output the
        printf("0x%02x ", *(unsigned char *)hwa_to_va(addr + i));
        if((i!=0)&&((i+1)%5==0)) 
        	printf("\n");
    }
    printf("\n");

    return 0;
}

static int cmd_si(char *args) {  //This part needs to fix with strtok() and atoi()

	char *pch;
	int infer=1;              //When the n is NULL so the debugger will execute one step
	pch=strtok(args," ");     //invoke strtok() function
	while(pch!=NULL)
	{
		infer=atoi(pch);//invoke atoi() function
		pch=strtok(NULL," ");
	}

	cpu_exec(infer);
    return 0;
}

static int cmd_p(char *p){
	bool success=true;

	int value=expr(p,&success);
	
	if (success==true) {
		print_token();
		printf("%u\n",value);
	}
}
static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },

	{ "si", "One Step Operate N commands until it End", cmd_si },
	{ "infor", "Print the Register Statue", cmd_info },
	{ "p", "Caculate the expression's expert", cmd_p },
	{ "x", "Caculate the expression's expert and take it as initial storage address as 0xffff", cmd_x },
//	{ "w", "When Expression changed to hold the program ", cmd_w },
//	{ "d", "Delete the number of N watch point", cmd_d },
//	{ "bt", "Print the stack frame chain", cmd_bt },


	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
